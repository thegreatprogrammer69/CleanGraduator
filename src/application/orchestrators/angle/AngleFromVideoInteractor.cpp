#include "AngleFromVideoInteractor.h"

#include <algorithm>
#include <exception>

#include "domain/ports/video/IVideoSource.h"
#include "domain/ports/angle/IAngleCalculator.h"

#include "domain/core/video/VideoFramePacket.h"
#include "domain/core/angle/AngleSourcePacket.h"
#include "domain/ports/angle/IAngleSink.h"

namespace application::orchestrators {

AngleFromVideoInteractor::AngleFromVideoInteractor(domain::common::AngleSourceId id,
                                                   AngleFromVideoInteractorPorts ports)
    : id_(id)
    , video_source_(ports.video_source)
    , anglemeter_(ports.anglemeter)
    , logger_(ports.logger)
{
    video_source_.addSink(*this);
    logger_.info("AngleFromVideoInteractor constructed");
}

AngleFromVideoInteractor::~AngleFromVideoInteractor() {
    stop();
    video_source_.addSink(*this);
    logger_.info("AngleFromVideoInteractor destroyed");
}

bool AngleFromVideoInteractor::isRunning() const noexcept {
    return state_.load(std::memory_order_acquire) == State::Started;
}

void AngleFromVideoInteractor::addSink(domain::ports::IAngleSink &s) {
    sinks_.add(s);
}

void AngleFromVideoInteractor::removeSink(domain::ports::IAngleSink &s) {
    sinks_.remove(s);
}

void AngleFromVideoInteractor::addObserver(domain::ports::IAngleSourceObserver &o) {
    observers_.add(o);
}

void AngleFromVideoInteractor::removeObserver(domain::ports::IAngleSourceObserver &o) {
    observers_.remove(o);
}


void AngleFromVideoInteractor::start() {
    state_.store(State::Started, std::memory_order_release);
}

void AngleFromVideoInteractor::stop() {
    state_.store(State::Stopped, std::memory_order_release);
}

void AngleFromVideoInteractor::onVideoFrame(const domain::common::VideoFramePacket& packet) {
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