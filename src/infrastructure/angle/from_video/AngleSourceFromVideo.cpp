#include "AngleSourceFromVideo.h"

#include <algorithm>
#include <cmath>
#include <exception>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "domain/ports/video/IVideoSource.h"
#include "domain/ports/angle/IAngleCalculator.h"

#include "domain/core/video/VideoFramePacket.h"
#include "domain/core/angle/AngleSourcePacket.h"
#include "domain/ports/angle/IAngleSink.h"

namespace infra::angle {
namespace {

// Быстрое включение/выключение дампа "плохих" кадров для дебага угла:
// 1) раскомментировать #define ниже;
// 2) пересобрать проект.
// Файлы сохраняются БЕЗ сжатия в ./bad_angle в формате PPM (P6).
#define ENABLE_BAD_ANGLE_FRAME_DUMP

bool isValidAngleMeasurement(const domain::common::Angle& angle)
{
    const double degrees = angle.degrees();
    return std::isfinite(degrees) && degrees >= 0.0 && degrees <= 360.0;
}

#ifdef ENABLE_BAD_ANGLE_FRAME_DUMP
void dumpBadAngleFrame(
    const domain::common::VideoFramePtr& frame,
    const domain::common::Timestamp& timestamp,
    const char* reason)
{
    if (!frame || frame->width <= 0 || frame->height <= 0 || !frame->buffer.data || frame->buffer.size == 0) {
        return;
    }

    if (frame->format != domain::common::PixelFormat::RGB24) {
        return;
    }

    const size_t expected_size = static_cast<size_t>(frame->width) * static_cast<size_t>(frame->height) * 3u;
    if (frame->buffer.size < expected_size) {
        return;
    }

    std::error_code ec;
    std::filesystem::create_directories("bad_angle", ec);
    if (ec) {
        return;
    }

    const auto ts_ms = static_cast<long long>(timestamp.asMilliseconds());
    std::ostringstream file_name;
    file_name << "bad_angle/"
              << "frame_" << ts_ms
              << "_" << reason
              << ".ppm";

    std::ofstream out(file_name.str(), std::ios::binary);
    if (!out) {
        return;
    }

    // PPM (P6): простой формат без сжатия.
    out << "P6\n" << frame->width << " " << frame->height << "\n255\n";
    out.write(reinterpret_cast<const char*>(frame->buffer.data), static_cast<std::streamsize>(expected_size));
}
#endif

} // namespace


AngleSourceFromVideo::AngleSourceFromVideo(
    domain::common::SourceId id,
    AngleSourcePorts ports,
    AngleSourceFromVideoConfig config)
    : id_(id)
    , video_source_(ports.video_source)
    , anglemeter_(ports.anglemeter)
    , logger_(ports.logger)
{
    video_source_.addSink(*this);
    logger_.info("AngleSourceFromVideo constructed");
}

AngleSourceFromVideo::~AngleSourceFromVideo() {
    stop();
    video_source_.removeSink(*this);
    logger_.info("AngleSourceFromVideo destroyed");
}

bool AngleSourceFromVideo::isRunning() const noexcept {
    return state_.load(std::memory_order_acquire) == State::Started;
}

void AngleSourceFromVideo::addSink(domain::ports::IAngleSink &s) {
    sinks_.add(s);
}

void AngleSourceFromVideo::removeSink(domain::ports::IAngleSink &s) {
    sinks_.remove(s);
}

void AngleSourceFromVideo::addObserver(domain::ports::IAngleSourceObserver &o) {
    observers_.add(o);
}

void AngleSourceFromVideo::removeObserver(domain::ports::IAngleSourceObserver &o) {
    observers_.remove(o);
}


void AngleSourceFromVideo::start() {
    state_.store(State::Started, std::memory_order_release);
}

void AngleSourceFromVideo::stop() {
    state_.store(State::Stopped, std::memory_order_release);
}

void AngleSourceFromVideo::onVideoFrame(const domain::common::VideoFramePacket& packet) {
    if (state_.load(std::memory_order_acquire) != State::Started) {
        return;
    }

    domain::common::AngleCalculatorInput input{};
    input.frame = packet.frame;

    domain::common::Angle angle{};
    try {
        angle = anglemeter_.calculate(input);
        // logger_.info("angle calculated: {}", angle);
    } catch (const std::exception& ex) {
#ifdef ENABLE_BAD_ANGLE_FRAME_DUMP
        dumpBadAngleFrame(packet.frame, packet.timestamp, "calc_exception");
#endif
        logger_.error("angle calc failed (ts={}): {}", packet.timestamp, ex.what());
        return;
    } catch (...) {
#ifdef ENABLE_BAD_ANGLE_FRAME_DUMP
        dumpBadAngleFrame(packet.frame, packet.timestamp, "calc_unknown_exception");
#endif
        logger_.error("angle calc failed (ts={}): unknown exception", packet.timestamp);
        return;
    }

    if (!isValidAngleMeasurement(angle)) {
#ifdef ENABLE_BAD_ANGLE_FRAME_DUMP
        dumpBadAngleFrame(packet.frame, packet.timestamp, "invalid_angle");
#endif
        logger_.error("invalid angle value: {}", angle);
        return;
    }

    domain::common::AngleSourcePacket out{};
    out.source_id = id_;
    out.timestamp = packet.timestamp;
    out.angle = angle;

    sinks_.notify([&out](domain::ports::IAngleSink& o) {
        o.onAnglePacket(out);
    });
}

} // namespace infra::angle
