#include "CalibrationLifecycle.h"

namespace infra::lifecycle {

CalibrationLifecycle::CalibrationLifecycle() = default;

// --------------------------------------------------
// State access
// --------------------------------------------------

domain::common::CalibrationLifecycleState
CalibrationLifecycle::state() const
{
    return state_;
}

std::string CalibrationLifecycle::lastError() const
{
    return last_error_;
}

// --------------------------------------------------
// Clock
// --------------------------------------------------

domain::ports::IClock& CalibrationLifecycle::sessionClock()
{
    return session_clock_;
}

// --------------------------------------------------
// Lifecycle transitions
// --------------------------------------------------

bool CalibrationLifecycle::start()
{
    if (state_ != State::Stopped)
        return false;

    last_error_.clear();

    state_ = State::Starting;
    notify();

    return true;
}

void CalibrationLifecycle::markRunning()
{
    if (state_ != State::Starting)
        return;

    state_ = State::Running;

    session_clock_.start();

    notify();
}

bool CalibrationLifecycle::stop()
{
    if (state_ != State::Running)
        return false;

    state_ = State::Stopping;
    notify();

    return true;
}

void CalibrationLifecycle::markIdle()
{
    if (state_ != State::Stopping)
        return;

    state_ = State::Idle;

    session_clock_.stop();

    notify();
}

void CalibrationLifecycle::markError(const std::string& err)
{
    // Error allowed only from active states
    if (state_ == State::Idle || state_ == State::Error)
        return;

    last_error_ = err;

    state_ = State::Error;

    session_clock_.stop();

    notify();
}

void CalibrationLifecycle::resetToIdle()
{
    if (state_ != State::Error)
        return;

    last_error_.clear();

    state_ = State::Idle;

    session_clock_.stop();

    notify();
}

// --------------------------------------------------
// Observers
// --------------------------------------------------

void CalibrationLifecycle::addObserver(
    domain::ports::ICalibrationLifecycleObserver& obs)
{
    if (std::find(observers_.begin(), observers_.end(), &obs) == observers_.end())
        observers_.push_back(&obs);
}

void CalibrationLifecycle::removeObserver(
    domain::ports::ICalibrationLifecycleObserver& obs)
{
    observers_.erase(
        std::remove(observers_.begin(), observers_.end(), &obs),
        observers_.end());
}

void CalibrationLifecycle::notify()
{
    for (auto* obs : observers_)
        obs->onCalibrationLifecycleStateChanged(state_, last_error_);
}

} // namespace infra::lifecycle