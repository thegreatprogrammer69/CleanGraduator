#include "AngleFromVideoInteractor.h"
#include "domain/ports/angle/IAngleCalculator.h"
#include "../../../domain/core/video/VideoFramePacket.h"
#include "domain/core/angle/AnglePacket.h"
#include "domain/ports/video/IVideoSource.h"
#include <algorithm>

namespace application::orchestrators {
    AngleFromVideoInteractor::AngleFromVideoInteractor(domain::common::AngleSourceId id, AngleFromVideoInteractorPorts ports)
        : id_(id)
        , video_source_(ports.video_source)
        , anglemeter_(ports.anglemeter)
        , logger_(ports.logger)
    {
        logger_.info("constructor called");
        video_source_.addObserver(*this);
    }

    AngleFromVideoInteractor::~AngleFromVideoInteractor() {
        video_source_.removeObserver(*this);
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

    bool AngleFromVideoInteractor::isRunning() const noexcept {
        return running_;
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
            if (!running_) {
                logger_.warn("frame ignored: interactor not running (ts={})", packet.timestamp);
                return;
            }
            localSinks = sinks_; // копия под защитой
        }

        domain::common::AngleCalculatorInput input;
        input.frame = packet.frame;

        domain::common::Angle angle;

        try {
            angle = anglemeter_.calculate(input);
        }
        catch (const std::exception& ex) {
            logger_.error("angle calculation failed (ts={}): {}", packet.timestamp, ex.what());
            return;
        }
        catch (...) {
            logger_.error("angle calculation failed (ts={}): unknown exception", packet.timestamp);
            return;
        }

        logger_.info("frame accepted, angle calculated (ts={}, angle={})",
                     packet.timestamp,
                     angle);

        domain::common::AnglePacket angle_packet;
        angle_packet.source_id = id_;
        angle_packet.timestamp = packet.timestamp;
        angle_packet.angle = angle;

        for (auto* sink : localSinks) {
            sink->onAngle(angle_packet);
        }
    }


    void AngleFromVideoInteractor::onVideoSourceOpened() {
    }

    void AngleFromVideoInteractor::onVideoSourceOpenFailed(const domain::common::VideoSourceError &) {
    }

    void AngleFromVideoInteractor::onVideoSourceClosed(const domain::common::VideoSourceError &) {

    }
} // namespace application
