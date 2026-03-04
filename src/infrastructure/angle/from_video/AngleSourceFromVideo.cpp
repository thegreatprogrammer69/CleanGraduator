#include "AngleSourceFromVideo.h"

#include <algorithm>
#include <exception>

#include "domain/ports/video/IVideoSource.h"
#include "domain/ports/angle/IAngleCalculator.h"

#include "domain/core/video/VideoFramePacket.h"
#include "domain/core/angle/AngleSourcePacket.h"
#include "domain/ports/angle/IAngleSink.h"

namespace infra::angle {

AngleSourceFromVideo::AngleSourceFromVideo(
    domain::common::AngleSourceId id,
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
    } catch (const std::exception& ex) {
        logger_.error("angle calc failed (ts={}): {}", packet.timestamp, ex.what());
        return;
    } catch (...) {
        logger_.error("angle calc failed (ts={}): unknown exception", packet.timestamp);
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

} // namespace application::orchestrators