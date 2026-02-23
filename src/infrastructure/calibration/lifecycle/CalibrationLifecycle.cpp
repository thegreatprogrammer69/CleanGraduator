#include "CalibrationLifecycle.h"

namespace infra::lifecycle {

CalibrationLifecycle::CalibrationLifecycle() = default;

// --------------------------------------------------
// State access
// --------------------------------------------------

domain::common::CalibrationLifecycleState
CalibrationLifecycle::state() const
{
    return _state;
}

std::string CalibrationLifecycle::lastError() const
{
    return _last_error;
}

// --------------------------------------------------
// Clock
// --------------------------------------------------

domain::ports::IClock& CalibrationLifecycle::sessionClock()
{
    return _session_clock;
}

// --------------------------------------------------
// Lifecycle transitions
// --------------------------------------------------

bool CalibrationLifecycle::start()
{
    if (_state != State::Idle)
        return false;

    _last_error.clear();

    _state = State::Starting;
    notify();

    return true;
}

void CalibrationLifecycle::markRunning()
{
    if (_state != State::Starting)
        return;

    _state = State::Running;

    _session_clock.start();

    notify();
}

bool CalibrationLifecycle::stop()
{
    if (_state != State::Running)
        return false;

    _state = State::Stopping;
    notify();

    return true;
}

void CalibrationLifecycle::markIdle()
{
    if (_state != State::Stopping)
        return;

    _state = State::Idle;

    _session_clock.stop();

    notify();
}

void CalibrationLifecycle::markError(const std::string& err)
{
    // Error allowed only from active states
    if (_state == State::Idle || _state == State::Error)
        return;

    _last_error = err;

    _state = State::Error;

    _session_clock.stop();

    notify();
}

void CalibrationLifecycle::resetToIdle()
{
    if (_state != State::Error)
        return;

    _last_error.clear();

    _state = State::Idle;

    _session_clock.stop();

    notify();
}

// --------------------------------------------------
// Observers
// --------------------------------------------------

void CalibrationLifecycle::addObserver(
    domain::ports::ICalibrationLifecycleObserver& obs)
{
    if (std::find(_observers.begin(), _observers.end(), &obs) == _observers.end())
        _observers.push_back(&obs);
}

void CalibrationLifecycle::removeObserver(
    domain::ports::ICalibrationLifecycleObserver& obs)
{
    _observers.erase(
        std::remove(_observers.begin(), _observers.end(), &obs),
        _observers.end());
}

void CalibrationLifecycle::notify()
{
    for (auto* obs : _observers)
        obs->onCalibrationLifecycleStateChanged(_state, _last_error);
}

} // namespace infra::lifecycle