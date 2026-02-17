#include "GStreamerCamera.h"

#include <cstring>
#include <mutex>
#include <algorithm>
#include <memory>

#include <gst/gst.h>
#include <gst/app/gstappsink.h>

#include "domain/core/measurement/Timestamp.h"
#include "domain/core/video/PixelFormat.h"
#include "domain/core/video/VideoFrame.h"
#include "domain/core/video/VideoFramePacket.h"
#include "domain/core/video/VideoSourceError.h"
#include "domain/ports/clock/IClock.h"

namespace infra::camera {

namespace {

std::once_flag g_gstInitOnce;

void ensureGstInit() {
    std::call_once(g_gstInitOnce, [] {
        int argc = 0;
        char** argv = nullptr;
        gst_init(&argc, &argv);
    });
}

GstElement* findAppSink(GstElement* pipeline) {
    if (!pipeline) return nullptr;

    // 1) Prefer explicit name
    if (auto* byName = gst_bin_get_by_name(GST_BIN(pipeline), "appsink")) {
        return byName; // ref++ (must unref later)
    }

    // 2) Otherwise scan all elements and find first appsink
    GstIterator* it = gst_bin_iterate_elements(GST_BIN(pipeline));
    if (!it) return nullptr;

    GValue item = G_VALUE_INIT;
    GstElement* found = nullptr;

    while (gst_iterator_next(it, &item) == GST_ITERATOR_OK) {
        auto* obj = static_cast<GstObject*>(g_value_get_object(&item));
        auto* elem = obj ? GST_ELEMENT(obj) : nullptr;

        if (elem && GST_IS_APP_SINK(elem)) {
            found = GST_ELEMENT(gst_object_ref(elem)); // take our own ref
            g_value_reset(&item);
            break;
        }
        g_value_reset(&item);
    }

    gst_iterator_free(it);
    return found;
}

bool readCapsWHF(GstSample* sample, int& w, int& h, const char*& fmtStr) {
    w = 0; h = 0; fmtStr = nullptr;

    if (!sample) return false;
    GstCaps* caps = gst_sample_get_caps(sample);
    if (!caps || gst_caps_is_empty(caps)) return false;

    GstStructure* s = gst_caps_get_structure(caps, 0);
    if (!s) return false;

    gst_structure_get_int(s, "width", &w);
    gst_structure_get_int(s, "height", &h);
    fmtStr = gst_structure_get_string(s, "format");
    return true;
}

domain::common::PixelFormat mapPixelFormat(const char* gstFormat) {
    if (!gstFormat) return domain::common::PixelFormat::YUYV;

    // GStreamer обычно называет YUYV как "YUY2"
    if (std::strcmp(gstFormat, "YUY2") == 0 || std::strcmp(gstFormat, "YUYV") == 0) {
        return domain::common::PixelFormat::YUYV;
    }

    if (std::strcmp(gstFormat, "RGB") == 0) {
        return domain::common::PixelFormat::RGB24;
    }

    // "RGB", "BGR", "GRAY8", "NV12", ...
    return domain::common::PixelFormat::YUYV;
}

void logBusMessages(fmt::Logger& logger, GstBus* bus, std::atomic<bool>& running) {
    if (!bus) return;

    for (;;) {
        GstMessage* msg = gst_bus_pop_filtered(
            bus,
            static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS)
        );
        if (!msg) break;

        if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_EOS) {
            logger.warn("GStreamer: EOS received");
            running.store(false);
        } else if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR) {
            GError* err = nullptr;
            gchar* debug = nullptr;
            gst_message_parse_error(msg, &err, &debug);

            logger.error("GStreamer error: {} (debug: {})",
                         err ? err->message : "unknown",
                         debug ? debug : "none");

            if (err) g_error_free(err);
            if (debug) g_free(debug);

            running.store(false);
        }

        gst_message_unref(msg);
    }
}

} // namespace

GStreamerCamera::GStreamerCamera(VideoSourcePorts ports, GStreamerCameraConfig config)
    : logger_(ports.logger), ports_(ports), config_(std::move(config)) {}

GStreamerCamera::~GStreamerCamera() {
    close();
}

bool GStreamerCamera::open() {
    auto abort_opening = [this]() {
        this->close();
        const domain::common::VideoSourceError err{logger_.lastError()};
        notifier_.notifyOpenFailed(err);
    };

    if (running_.load()) return false;

    ensureGstInit();

    if (config_.pipe.empty()) {
        logger_.error("GStreamerCamera: empty pipeline string");
        throw std::runtime_error(logger_.lastError());
    }

    GError* error = nullptr;
    pipeline_ = gst_parse_launch(config_.pipe.c_str(), &error);
    if (!pipeline_) {
        logger_.error("GStreamerCamera: gst_parse_launch failed: {}",
                      error ? error->message : "unknown");
        if (error) g_error_free(error);
        abort_opening(); return false;
    }
    if (error) {
        // parse_launch может вернуть pipeline и warning/error одновременно
        logger_.warn("GStreamerCamera: gst_parse_launch message: {}", error->message);
        g_error_free(error);
        error = nullptr;
    }

    appsink_ = findAppSink(pipeline_);
    if (!appsink_) {
        logger_.error("GStreamerCamera: appsink not found. "
                      "Pipeline must contain appsink (preferably name=appsink).");
        abort_opening(); return false;
    }

    // Настроим appsink (можно также делать это в строке pipe через свойства)
    gst_app_sink_set_emit_signals(GST_APP_SINK(appsink_), FALSE);
    gst_app_sink_set_drop(GST_APP_SINK(appsink_), TRUE);
    gst_app_sink_set_max_buffers(GST_APP_SINK(appsink_), 2);

    // Чтобы кадры не ждали синхронизации по clock:
    g_object_set(G_OBJECT(appsink_), "sync", FALSE, nullptr);

    bus_ = gst_element_get_bus(pipeline_);

    GstStateChangeReturn ret = gst_element_set_state(pipeline_, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        logger_.error("GStreamerCamera: failed to set pipeline to PLAYING");
        abort_opening(); return false;
    }

    // Дождёмся перехода в PLAYING (коротко, чтобы понять что всё ок)
    GstState state = GST_STATE_NULL;
    ret = gst_element_get_state(pipeline_, &state, nullptr, 2 * GST_SECOND);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        logger_.error("GStreamerCamera: pipeline failed to reach PLAYING");
        abort_opening(); return false;
    }

    running_.store(true);
    thread_ = std::thread(&GStreamerCamera::captureLoop, this);

    logger_.info("GStreamerCamera started: {}", config_.pipe);

    notifier_.notifyOpened();
    return true;
}

void GStreamerCamera::close() {
    if (!running_.exchange(false)) return;

    // Попросим pipeline остановиться до join — так thread быстрее выйдет из pull_sample
    if (pipeline_) {
        gst_element_set_state(pipeline_, GST_STATE_NULL);
    }

    if (thread_.joinable()) {
        thread_.join();
    }

    if (bus_) {
        gst_object_unref(bus_);
        bus_ = nullptr;
    }

    if (appsink_) {
        gst_object_unref(appsink_);
        appsink_ = nullptr;
    }

    if (pipeline_) {
        gst_object_unref(pipeline_);
        pipeline_ = nullptr;
    }

    notifier_.notifyClosed({});
}

void GStreamerCamera::addObserver(domain::ports::IVideoSourceObserver &o) {
    notifier_.addObserver(o);
}

void GStreamerCamera::removeObserver(domain::ports::IVideoSourceObserver &o) {
    notifier_.removeObserver(o);
}

void GStreamerCamera::captureLoop() {
    // Pull samples with timeout so stop() is responsive
    const GstClockTime timeout = 200 * GST_MSECOND;

    while (running_.load()) {
        // bus error/eos
        logBusMessages(logger_, bus_, running_);
        if (!running_.load()) break;

        GstSample* sample = gst_app_sink_try_pull_sample(GST_APP_SINK(appsink_), timeout);
        if (!sample) {
            // timeout (or flushing) — просто продолжим
            continue;
        }

        dispatchSample(sample);
        gst_sample_unref(sample);
    }

    notifier_.notifyClosed({});
}

void GStreamerCamera::dispatchSample(GstSample* sample) {
    if (!sample) return;

    GstBuffer* buffer = gst_sample_get_buffer(sample);
    if (!buffer) return;

    GstMapInfo map{};
    if (!gst_buffer_map(buffer, &map, GST_MAP_READ)) {
        logger_.warn("GStreamerCamera: gst_buffer_map failed");
        return;
    }

    int w = 0, h = 0;
    const char* fmtStr = nullptr;
    (void)readCapsWHF(sample, w, h, fmtStr);

    auto frame = std::make_shared<domain::common::VideoFrame>();
    frame->width  = w;
    frame->height = h;
    frame->format = mapPixelFormat(fmtStr);
    frame->buffer = domain::common::VideoBuffer(static_cast<int>(map.size));

    std::memcpy(frame->buffer.data, map.data, map.size);
    gst_buffer_unmap(buffer, &map);

    const domain::common::Timestamp ts = ports_.clock.now();

    domain::common::VideoFramePacket packet;
    packet.timestamp = ts;
    packet.frame = frame;

    notifier_.notifyFrame(packet);
}

} // namespace infra::camera
