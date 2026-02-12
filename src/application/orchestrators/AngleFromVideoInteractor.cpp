#include "AngleFromVideoInteractor.h"
#include "domain/ports/inbound/IAngleCalculator.h"
#include <algorithm>

#include "domain/core/video/VideoFramePacket.h"


namespace application::interactors {
    AngleFromVideoInteractor::AngleFromVideoInteractor(AngleFromVideoInteractorPorts ports)
        : anglemeter_(ports.anglemeter)
        , logger_(ports.logger)
    {
        logger_.info("constructor called");
    }

    void AngleFromVideoInteractor::start() {
        {
            std::lock_guard lock(mutex_);
            running_ = true;
        }
        logger_.info("started");
    }

    void AngleFromVideoInteractor::stop() {
        {
            std::lock_guard lock(mutex_);
            running_ = false;
        }
        logger_.info("stopped");
    }

    void AngleFromVideoInteractor::addSink(domain::ports::IAngleSink& sink) {
        {
            std::lock_guard lock(mutex_);
            if (std::find(sinks_.begin(), sinks_.end(), &sink) == sinks_.end()) {
                sinks_.push_back(&sink);
            }
        }

        logger_.info("sink added");
    }

    void AngleFromVideoInteractor::removeSink(domain::ports::IAngleSink& sink) {
        {
            std::lock_guard lock(mutex_);
            sinks_.erase(
                std::remove(sinks_.begin(), sinks_.end(), &sink),
                sinks_.end()
            );
        }

        logger_.info("sink removed");
    }

    void AngleFromVideoInteractor::onVideoFrame(const domain::common::VideoFramePacket& packet) {
        std::vector<domain::ports::IAngleSink*> localSinks;

        {
            std::lock_guard lock(mutex_);
            if (!running_) return;
            localSinks = sinks_; // копия под защитой
        }

        domain::common::AnglemeterInput input;
        input.frame = packet.frame;
        const auto angle = anglemeter_.calculate(input);

        logger_.info("frame accepted, angle calculated (ts={}, angle={})", packet.timestamp, angle);

        for (auto* sink : localSinks) {
            sink->onAngle(packet.timestamp, angle);
        }
    }

} // namespace application
