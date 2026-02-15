#include "Motor.h"
#include "domain/ports/motor/IMotorDriver.h"

using namespace domain::common;
using namespace domain::ports;

Motor::Motor(IMotorDriver& driver,
             int minFrequency,
             int maxFrequency)
    : driver_(driver),
      minFrequency_(minFrequency),
      maxFrequency_(maxFrequency)
{
    driver_.addObserver(*this);
}

Motor::~Motor()
{
    driver_.removeObserver(*this);
}

// --------------------
// State getters
// --------------------

MotorState Motor::state() const noexcept {
    std::lock_guard lock(mutex_);
    return state_;
}

bool Motor::isRunning() const noexcept {
    std::lock_guard lock(mutex_);
    return state_ == MotorState::RunningForward ||
           state_ == MotorState::RunningBackward;
}

int Motor::currentFrequency() const noexcept {
    std::lock_guard lock(mutex_);
    return currentFrequency_;
}

MotorFault Motor::fault() const noexcept {
    std::lock_guard lock(mutex_);
    return fault_;
}

// --------------------
// Commands
// --------------------

void Motor::startForward(int hz) {
    std::lock_guard lock(mutex_);

    ensureNotEmergency();
    ensureNotRunning();
    ensureValidFrequency(hz);
    checkForwardLimit();

    driver_.setDirection(MotorDirection::Forward);
    driver_.setFrequency(hz);
    driver_.start();

    state_ = MotorState::RunningForward;
    currentFrequency_ = hz;
}

void Motor::startBackward(int hz) {
    std::lock_guard lock(mutex_);

    ensureNotEmergency();
    ensureNotRunning();
    ensureValidFrequency(hz);
    checkBackwardLimit();

    driver_.setDirection(MotorDirection::Backward);
    driver_.setFrequency(hz);
    driver_.start();

    state_ = MotorState::RunningBackward;
    currentFrequency_ = hz;
}

void Motor::changeFrequency(int hz) {
    std::lock_guard lock(mutex_);

    ensureRunning();
    ensureValidFrequency(hz);

    driver_.setFrequency(hz);
    currentFrequency_ = hz;
}

void Motor::stop() {
    std::lock_guard lock(mutex_);

    if (!isRunning())
        return;

    driver_.stop();
    state_ = MotorState::Idle;
    currentFrequency_ = 0;
}

void Motor::emergencyStop() {
    std::lock_guard lock(mutex_);

    driver_.emergencyStop();
    state_ = MotorState::EmergencyStopped;
    currentFrequency_ = 0;
}

void Motor::resetEmergency() {
    std::lock_guard lock(mutex_);

    if (state_ != MotorState::EmergencyStopped)
        return;

    fault_ = MotorFault::None;
    state_ = MotorState::Idle;
}

// --------------------
// Driver callbacks
// --------------------

void Motor::onMotorStopped() {
    std::lock_guard lock(mutex_);

    state_ = MotorState::Idle;
    currentFrequency_ = 0;
}

void Motor::onMotorEmergencyStop() {
    std::lock_guard lock(mutex_);

    state_ = MotorState::EmergencyStopped;
    currentFrequency_ = 0;
}

void Motor::onMotorFault(MotorFault fault) {
    std::lock_guard lock(mutex_);

    fault_ = fault;
    state_ = MotorState::EmergencyStopped;
    currentFrequency_ = 0;
}

// --------------------
// Guards
// --------------------

void Motor::ensureNotEmergency() const {
    if (state_ == MotorState::EmergencyStopped)
        throw std::logic_error("Motor is in emergency state");
}

void Motor::ensureNotRunning() const {
    if (state_ == MotorState::RunningForward ||
        state_ == MotorState::RunningBackward)
        throw std::logic_error("Motor already running");
}

void Motor::ensureRunning() const {
    if (state_ != MotorState::RunningForward &&
        state_ != MotorState::RunningBackward)
        throw std::logic_error("Motor is not running");
}

void Motor::ensureValidFrequency(int hz) const {
    if (hz < minFrequency_ || hz > maxFrequency_)
        throw std::invalid_argument("Frequency out of range");
}

void Motor::checkForwardLimit() const {
    auto limits = driver_.limits();
    if (limits.end)
        throw std::logic_error("Forward limit reached");
}

void Motor::checkBackwardLimit() const {
    auto limits = driver_.limits();
    if (limits.home)
        throw std::logic_error("Backward limit reached");
}
