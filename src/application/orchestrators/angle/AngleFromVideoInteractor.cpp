#include "AngleFromVideoInteractor.h"

#include <algorithm>
#include <exception>

#include "domain/ports/video/IVideoSource.h"
#include "domain/ports/angle/IAngleCalculator.h"

#include "domain/core/video/VideoFramePacket.h"
#include "domain/core/angle/AngleSourcePacket.h"

namespace application::orchestrators {

AngleFromVideoInteractor::AngleFromVideoInteractor(domain::common::AngleSourceId id,
                                                   AngleFromVideoInteractorPorts ports)
    : id_(id)
    , video_source_(ports.video_source)
    , anglemeter_(ports.anglemeter)
    , logger_(ports.logger)
{
    video_source_.addObserver(*this);
    logger_.info("AngleFromVideoInteractor constructed");
}

AngleFromVideoInteractor::~AngleFromVideoInteractor() {
    stop(); // идемпотентно
    video_source_.removeObserver(*this);
    logger_.info("AngleFromVideoInteractor destroyed");
}

bool AngleFromVideoInteractor::isRunning() const noexcept {
    return state_.load(std::memory_order_acquire) == State::Running;
}

void AngleFromVideoInteractor::start() {
    State s = state_.load(std::memory_order_acquire);
    if (s == State::Starting || s == State::Running) {
        return;
    }

    state_.store(State::Starting, std::memory_order_release);
    logger_.info("start(): opening video source");

    try {
        video_source_.open();
    } catch (const std::exception& ex) {
        logger_.error("start(): open() threw: {}", ex.what());
        notifyFailed_("video open threw exception");
        state_.store(State::Stopped, std::memory_order_release);
        notifyStopped_();
    } catch (...) {
        logger_.error("start(): open() threw unknown exception");
        notifyFailed_("video open threw unknown exception");
        state_.store(State::Stopped, std::memory_order_release);
        notifyStopped_();
    }
}

void AngleFromVideoInteractor::stop() {
    State s = state_.load(std::memory_order_acquire);
    if (s == State::Stopped || s == State::Stopping) {
        return;
    }

    state_.store(State::Stopping, std::memory_order_release);
    logger_.info("stop(): closing video source");

    try {
        video_source_.close();
    } catch (const std::exception& ex) {
        logger_.error("stop(): close() threw: {}", ex.what());
        // закрыться не смогли — но состояние источника для нас завершаем
        state_.store(State::Stopped, std::memory_order_release);
        notifyStopped_();
    } catch (...) {
        logger_.error("stop(): close() threw unknown exception");
        state_.store(State::Stopped, std::memory_order_release);
        notifyStopped_();
    }
}

void AngleFromVideoInteractor::addObserver(domain::ports::IAngleSourceObserver& sink) {
    bool shouldNotifyStarted = false;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = std::find(observers_.begin(), observers_.end(), &sink);
        if (it == observers_.end()) {
            observers_.push_back(&sink);
        }
        shouldNotifyStarted = (state_.load(std::memory_order_acquire) == State::Running);
    }

    if (shouldNotifyStarted) {
        sink.onAngleSourceStarted();
    }
}

void AngleFromVideoInteractor::removeObserver(domain::ports::IAngleSourceObserver& sink) {
    std::lock_guard<std::mutex> lock(mutex_);
    observers_.erase(std::remove(observers_.begin(), observers_.end(), &sink), observers_.end());
}

void AngleFromVideoInteractor::onVideoSourceOpened() {
    State s = state_.load(std::memory_order_acquire);

    if (s == State::Starting) {
        state_.store(State::Running, std::memory_order_release);
        logger_.info("video opened -> Running");
        notifyStarted_();
        return;
    }

    if (s == State::Stopping) {
        logger_.warn("video opened while stopping -> closing");
        try { video_source_.close(); } catch (...) {}
        return;
    }

    // Running/Stopped — игнорируем
    logger_.warn("onVideoSourceOpened(): unexpected state");
}

void AngleFromVideoInteractor::onVideoSourceFailed(const domain::common::VideoSourceError& /*err*/) {
    State s = state_.load(std::memory_order_acquire);
    if (s == State::Stopped) {
        return;
    }

    logger_.error("video source failed");
    notifyFailed_("video source failed");

    state_.store(State::Stopped, std::memory_order_release);
    notifyStopped_();
}

void AngleFromVideoInteractor::onVideoSourceClosed() {
    State prev = state_.exchange(State::Stopped, std::memory_order_acq_rel);
    if (prev == State::Stopped) {
        return;
    }

    logger_.info("video closed -> Stopped");
    notifyStopped_();
}

void AngleFromVideoInteractor::onVideoFrame(const domain::common::VideoFramePacket& packet) {
    if (state_.load(std::memory_order_acquire) != State::Running) {
        return;
    }

    std::vector<domain::ports::IAngleSourceObserver*> sinks;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        sinks = observers_;
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

    for (auto* s : sinks) {
        if (s) {
            s->onAngleSourcePacket(out);
        }
    }
}

// ----- notifications (без локов во время callback'ов) -----

void AngleFromVideoInteractor::notifyStarted_() {
    std::vector<domain::ports::IAngleSourceObserver*> sinks;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        sinks = observers_;
    }
    for (auto* s : sinks) {
        if (s) s->onAngleSourceStarted();
    }
}

void AngleFromVideoInteractor::notifyStopped_() {
    std::vector<domain::ports::IAngleSourceObserver*> sinks;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        sinks = observers_;
    }
    for (auto* s : sinks) {
        if (s) s->onAngleSourceStopped();
    }
}

void AngleFromVideoInteractor::notifyFailed_(const char* msg) {
    domain::common::AngleSourceError e{};
    e.id = id_;
    e.error = msg;

    std::vector<domain::ports::IAngleSourceObserver*> sinks;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        sinks = observers_;
    }
    for (auto* s : sinks) {
        if (s) s->onAngleSourceFailed(e);
    }
}

} // namespace application::orchestrators