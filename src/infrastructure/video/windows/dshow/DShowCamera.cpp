#include "DShowCamera.h"

#include "SampleGrabberCB.h"

#include "domain/core/video/VideoFrame.h"
#include "domain/core/video/VideoFramePacket.h"
#include "domain/core/video/VideoSourceError.h"
#include "domain/core/video/VideoSourceEvent.h"
#include "domain/ports/clock/IClock.h"

#include <dshow.h>
#include <windows.h>
#include <wrl/client.h>
#include <atlbase.h>
#include <atlcom.h>

#include <chrono>
#include <cstring>
#include <utility>

#pragma comment(lib, "strmiids.lib")

using namespace infra::camera;
using namespace domain::common;

namespace {

CComPtr<IBaseFilter> CreateCaptureFilterByIndex(int index) {
    CComPtr<ICreateDevEnum> dev_enum;
    CComPtr<IEnumMoniker> enum_moniker;

    HRESULT hr = dev_enum.CoCreateInstance(CLSID_SystemDeviceEnum);
    if (FAILED(hr)) {
        return nullptr;
    }

    hr = dev_enum->CreateClassEnumerator(
        CLSID_VideoInputDeviceCategory,
        &enum_moniker,
        0
    );
    if (FAILED(hr) || hr == S_FALSE) {
        return nullptr;
    }

    ULONG fetched = 0;
    CComPtr<IMoniker> moniker;
    int i = 0;

    while (enum_moniker->Next(1, &moniker, &fetched) == S_OK) {
        if (i == index) {
            CComPtr<IBaseFilter> filter;
            if (SUCCEEDED(moniker->BindToObject(
                    nullptr,
                    nullptr,
                    IID_IBaseFilter,
                    reinterpret_cast<void**>(&filter)))) {
                return filter;
            }
            break;
        }

        moniker.Release();
        ++i;
    }

    return nullptr;
}

inline long long nowSteadyMs() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(
        steady_clock::now().time_since_epoch()
    ).count();
}

} // namespace

struct DShowCamera::DShowCameraImpl {
    CComPtr<IGraphBuilder> graph;
    CComPtr<ICaptureGraphBuilder2> capture;
    CComPtr<IMediaControl> control;

    CComPtr<IBaseFilter> source_filter;
    CComPtr<IBaseFilter> grabber_filter;
    CComPtr<ISampleGrabber> grabber;
    CComPtr<IBaseFilter> null_renderer;

    SampleGrabberCB* grabber_cb = nullptr;

    int width = 0;
    int height = 0;

    bool com_initialized = false;
    bool running = false;

    long long last_frame_tick_ms = 0;

    std::chrono::milliseconds check_period{500};
    std::chrono::milliseconds timeout{1500};

    ~DShowCameraImpl() {
        if (control) {
            control->Stop();
        }

        if (grabber_cb) {
            grabber_cb->Release();
            grabber_cb = nullptr;
        }

        graph.Release();
        capture.Release();
        control.Release();
        grabber.Release();
        grabber_filter.Release();
        source_filter.Release();
        null_renderer.Release();

        if (com_initialized) {
            // Разкомментируй, если open()/close() гарантированно на одном и том же thread.
            // CoUninitialize();
        }
    }
};

DShowCamera::DShowCamera(VideoSourcePorts ports, DShowCameraConfig config)
    : logger_(ports.logger)
    , clock_(ports.clock)
    , config_(config)
    , watchdog_worker_([this]() { watchdogTick(); })
{
    logger_.info("DShowCamera ctor: camera index={}", config_.index);
}

DShowCamera::~DShowCamera() {
    close();
    logger_.info("DShowCamera dtor");
}

void DShowCamera::open() {
    std::lock_guard<std::mutex> lock(state_mutex_);

    if (impl_ && impl_->running) {
        logger_.warn("DShowCamera::open() ignored: already running");
        return;
    }

    logger_.info("DShowCamera::open() begin");

    impl_ = std::make_unique<DShowCameraImpl>();

    if (!initializeCom())       return;
    if (!createGraph())         return;
    if (!createSourceFilter())  return;
    if (!createSampleGrabber()) return;
    if (!createNullRenderer())  return;
    if (!connectGraph())        return;
    if (!readConnectedFormat()) return;
    if (!configureSampleGrabber()) return;
    if (!runGraph())            return;

    impl_->last_frame_tick_ms = nowSteadyMs();
    impl_->running = true;

    startWatchdog();

    logger_.info("DShowCamera opened successfully");
    notifier_.notifyEvent(VideoSourceEvent(VideoSourceEvent::Opened{}));
}

void DShowCamera::close() {
    stopWatchdog();

    std::lock_guard<std::mutex> lock(state_mutex_);

    if (!impl_) {
        return;
    }

    logger_.info("DShowCamera::close()");

    impl_->running = false;
    impl_.reset();

    notifier_.notifyEvent(VideoSourceEvent(VideoSourceEvent::Closed{}));
    logger_.info("DShowCamera closed");
}

bool DShowCamera::isRunning() const noexcept {
    std::lock_guard<std::mutex> lock(state_mutex_);
    return impl_ != nullptr && impl_->running;
}

void DShowCamera::addObserver(domain::ports::IVideoSourceObserver& o) {
    notifier_.addObserver(o);
}

void DShowCamera::removeObserver(domain::ports::IVideoSourceObserver& o) {
    notifier_.removeObserver(o);
}

void DShowCamera::addSink(domain::ports::IVideoSink& s) {
    notifier_.addSink(s);
}

void DShowCamera::removeSink(domain::ports::IVideoSink& s) {
    notifier_.removeSink(s);
}

void DShowCamera::onFrame(double /*time*/, unsigned char* data, long size) {
    if (data == nullptr || size <= 0) {
        return;
    }

    int width = 0;
    int height = 0;

    {
        std::lock_guard<std::mutex> lock(state_mutex_);

        if (!impl_ || !impl_->running) {
            return;
        }

        impl_->last_frame_tick_ms = nowSteadyMs();
        width = impl_->width;
        height = impl_->height;
    }

    auto frame = std::make_shared<VideoFrame>();
    frame->width = width;
    frame->height = height;
    frame->format = PixelFormat::RGB24;
    frame->buffer = VideoBuffer(size);

    std::memcpy(frame->buffer.data, data, static_cast<size_t>(size));

    VideoFramePacket packet;
    packet.frame = std::move(frame);
    packet.timestamp = clock_.now();

    notifier_.notifyFrame(packet);
}

bool DShowCamera::initializeCom() {
    const HRESULT hr = CoInitializeEx(
        nullptr,
        COINIT_MULTITHREADED | COINIT_SPEED_OVER_MEMORY
    );

    if (FAILED(hr)) {
        failOpen(fmt::format("CoInitializeEx failed: hr=0x{:08X}", static_cast<unsigned>(hr)));
        return false;
    }

    impl_->com_initialized = true;
    logger_.info("COM initialized");
    return true;
}

bool DShowCamera::createGraph() {
    HRESULT hr = impl_->graph.CoCreateInstance(CLSID_FilterGraph);
    if (FAILED(hr)) {
        failOpen(fmt::format("Create FilterGraph failed: hr=0x{:08X}", static_cast<unsigned>(hr)));
        return false;
    }

    hr = impl_->capture.CoCreateInstance(CLSID_CaptureGraphBuilder2);
    if (FAILED(hr)) {
        failOpen(fmt::format("Create CaptureGraphBuilder2 failed: hr=0x{:08X}", static_cast<unsigned>(hr)));
        return false;
    }

    hr = impl_->capture->SetFiltergraph(impl_->graph);
    if (FAILED(hr)) {
        failOpen(fmt::format("SetFiltergraph failed: hr=0x{:08X}", static_cast<unsigned>(hr)));
        return false;
    }

    hr = impl_->graph.QueryInterface(&impl_->control);
    if (FAILED(hr) || !impl_->control) {
        failOpen(fmt::format("Query IMediaControl failed: hr=0x{:08X}", static_cast<unsigned>(hr)));
        return false;
    }

    logger_.info("Graph created");
    return true;
}

bool DShowCamera::createSourceFilter() {
    impl_->source_filter = CreateCaptureFilterByIndex(config_.index);
    if (!impl_->source_filter) {
        failOpen(fmt::format("Device not found, index={}", config_.index));
        return false;
    }

    const HRESULT hr = impl_->graph->AddFilter(impl_->source_filter, L"Video Source");
    if (FAILED(hr)) {
        failOpen(fmt::format("Add source filter failed: hr=0x{:08X}", static_cast<unsigned>(hr)));
        return false;
    }

    logger_.info("Source filter added");
    return true;
}

bool DShowCamera::createSampleGrabber() {
    HRESULT hr = impl_->grabber_filter.CoCreateInstance(CLSID_SampleGrabber);
    if (FAILED(hr)) {
        failOpen(fmt::format("Create SampleGrabber filter failed: hr=0x{:08X}", static_cast<unsigned>(hr)));
        return false;
    }

    hr = impl_->grabber_filter.QueryInterface(&impl_->grabber);
    if (FAILED(hr) || !impl_->grabber) {
        failOpen(fmt::format("Query ISampleGrabber failed: hr=0x{:08X}", static_cast<unsigned>(hr)));
        return false;
    }

    AM_MEDIA_TYPE mt{};
    mt.majortype = MEDIATYPE_Video;
    mt.subtype = MEDIASUBTYPE_RGB24;
    mt.formattype = FORMAT_VideoInfo;

    hr = impl_->grabber->SetMediaType(&mt);
    if (FAILED(hr)) {
        failOpen(fmt::format("SetMediaType failed: hr=0x{:08X}", static_cast<unsigned>(hr)));
        return false;
    }

    hr = impl_->graph->AddFilter(impl_->grabber_filter, L"SampleGrabber");
    if (FAILED(hr)) {
        failOpen(fmt::format("Add SampleGrabber failed: hr=0x{:08X}", static_cast<unsigned>(hr)));
        return false;
    }

    logger_.info("SampleGrabber created");
    return true;
}

bool DShowCamera::createNullRenderer() {
    HRESULT hr = impl_->null_renderer.CoCreateInstance(CLSID_NullRenderer);
    if (FAILED(hr)) {
        failOpen(fmt::format("Create NullRenderer failed: hr=0x{:08X}", static_cast<unsigned>(hr)));
        return false;
    }

    hr = impl_->graph->AddFilter(impl_->null_renderer, L"NullRenderer");
    if (FAILED(hr)) {
        failOpen(fmt::format("Add NullRenderer failed: hr=0x{:08X}", static_cast<unsigned>(hr)));
        return false;
    }

    logger_.info("NullRenderer added");
    return true;
}

bool DShowCamera::connectGraph() {
    const HRESULT hr = impl_->capture->RenderStream(
        &PIN_CATEGORY_CAPTURE,
        &MEDIATYPE_Video,
        impl_->source_filter,
        impl_->grabber_filter,
        impl_->null_renderer
    );

    if (FAILED(hr)) {
        failOpen(fmt::format("RenderStream failed: hr=0x{:08X}", static_cast<unsigned>(hr)));
        return false;
    }

    logger_.info("Graph connected");
    return true;
}

bool DShowCamera::readConnectedFormat() {
    AM_MEDIA_TYPE mt_out{};
    const HRESULT hr = impl_->grabber->GetConnectedMediaType(&mt_out);

    if (FAILED(hr)) {
        failOpen(fmt::format("GetConnectedMediaType failed: hr=0x{:08X}", static_cast<unsigned>(hr)));
        return false;
    }

    if (mt_out.formattype != FORMAT_VideoInfo ||
        mt_out.pbFormat == nullptr ||
        mt_out.cbFormat < sizeof(VIDEOINFOHEADER)) {

        if (mt_out.pbFormat) {
            CoTaskMemFree(mt_out.pbFormat);
        }
        if (mt_out.pUnk) {
            mt_out.pUnk->Release();
        }

        failOpen("Unexpected media type from SampleGrabber");
        return false;
    }

    const auto* vih = reinterpret_cast<const VIDEOINFOHEADER*>(mt_out.pbFormat);
    impl_->width = vih->bmiHeader.biWidth;
    impl_->height = vih->bmiHeader.biHeight;

    CoTaskMemFree(mt_out.pbFormat);
    if (mt_out.pUnk) {
        mt_out.pUnk->Release();
    }

    logger_.info("Video format: {}x{}", impl_->width, impl_->height);
    return true;
}

bool DShowCamera::configureSampleGrabber() {
    HRESULT hr = impl_->grabber->SetBufferSamples(FALSE);
    if (FAILED(hr)) {
        failOpen(fmt::format("SetBufferSamples failed: hr=0x{:08X}", static_cast<unsigned>(hr)));
        return false;
    }

    hr = impl_->grabber->SetOneShot(FALSE);
    if (FAILED(hr)) {
        failOpen(fmt::format("SetOneShot failed: hr=0x{:08X}", static_cast<unsigned>(hr)));
        return false;
    }

    impl_->grabber_cb = new SampleGrabberCB(this);

    hr = impl_->grabber->SetCallback(impl_->grabber_cb, 1);
    if (FAILED(hr)) {
        failOpen(fmt::format("SetCallback failed: hr=0x{:08X}", static_cast<unsigned>(hr)));
        return false;
    }

    logger_.info("SampleGrabber callback configured");
    return true;
}

bool DShowCamera::runGraph() {
    const HRESULT hr = impl_->control->Run();
    if (FAILED(hr)) {
        failOpen(fmt::format("Graph Run failed: hr=0x{:08X}", static_cast<unsigned>(hr)));
        return false;
    }

    logger_.info("Graph started");
    return true;
}

void DShowCamera::startWatchdog() {
    watchdog_enabled_.store(true, std::memory_order_release);
    watchdog_worker_.start();
    logger_.info("Watchdog started");
}

void DShowCamera::stopWatchdog() {
    watchdog_enabled_.store(false, std::memory_order_release);
    watchdog_worker_.stop();
}

void DShowCamera::watchdogTick() {
    using namespace std::chrono_literals;

    std::this_thread::sleep_for(500ms);

    if (!watchdog_enabled_.load(std::memory_order_acquire)) {
        return;
    }

    bool timeout_detected = false;

    {
        std::lock_guard<std::mutex> lock(state_mutex_);

        if (!impl_ || !impl_->running) {
            return;
        }

        const long long elapsed_ms = nowSteadyMs() - impl_->last_frame_tick_ms;
        if (elapsed_ms > impl_->timeout.count()) {
            impl_->running = false;
            timeout_detected = true;
        }
    }

    if (timeout_detected) {
        watchdog_enabled_.store(false, std::memory_order_release);
        failRuntime("Video stream timeout (no frames)");
    }
}

void DShowCamera::failOpen(const std::string& reason) {
    logger_.error(reason);

    VideoSourceEvent::OpenFailed ev;
    ev.error = VideoSourceError{reason};

    impl_.reset();
    notifier_.notifyEvent(VideoSourceEvent(ev));
}

void DShowCamera::failRuntime(const std::string& reason) {
    logger_.error(reason);

    VideoSourceEvent::Failed ev;
    ev.error = VideoSourceError{reason};

    notifier_.notifyEvent(VideoSourceEvent(ev));
}