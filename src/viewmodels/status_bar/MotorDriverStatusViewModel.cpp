#include "MotorDriverStatusViewModel.h"

#include "domain/ports/motor/IMotorDriver.h"

mvvm::MotorDriverStatusViewModel::MotorDriverStatusViewModel(MotorDriverStatusViewModelDeps deps)
    : motor_driver_(deps.motor_driver)
{
    motor_driver_.addObserver(*this);

    const auto current_limits = motor_driver_.limits();
    home_limit_.store(current_limits.home, std::memory_order_relaxed);
    end_limit_.store(current_limits.end, std::memory_order_relaxed);
    fault_.store(motor_driver_.fault(), std::memory_order_relaxed);
}

mvvm::MotorDriverStatusViewModel::~MotorDriverStatusViewModel() {
    motor_driver_.removeObserver(*this);
}

bool mvvm::MotorDriverStatusViewModel::isRunning() const {
    return is_running_.load(std::memory_order_relaxed);
}

int mvvm::MotorDriverStatusViewModel::frequencyHz() const {
    return frequency_hz_.load(std::memory_order_relaxed);
}

domain::common::MotorDirection mvvm::MotorDriverStatusViewModel::direction() const {
    return direction_.load(std::memory_order_relaxed);
}

domain::common::MotorLimitsState mvvm::MotorDriverStatusViewModel::limits() const {
    return {
        home_limit_.load(std::memory_order_relaxed),
        end_limit_.load(std::memory_order_relaxed)
    };
}

domain::common::MotorFault mvvm::MotorDriverStatusViewModel::fault() const {
    return fault_.load(std::memory_order_relaxed);
}

void mvvm::MotorDriverStatusViewModel::onStarted() {
    is_running_.store(true, std::memory_order_relaxed);
}

void mvvm::MotorDriverStatusViewModel::onStopped() {
    is_running_.store(false, std::memory_order_relaxed);
    frequency_hz_.store(0, std::memory_order_relaxed);
}

void mvvm::MotorDriverStatusViewModel::onFrequencyChanged(int hz) {
    frequency_hz_.store(hz, std::memory_order_relaxed);
}

void mvvm::MotorDriverStatusViewModel::onDirectionChanged(domain::common::MotorDirection dir) {
    direction_.store(dir, std::memory_order_relaxed);
}

void mvvm::MotorDriverStatusViewModel::onLimitsChanged(domain::common::MotorLimitsState state) {
    home_limit_.store(state.home, std::memory_order_relaxed);
    end_limit_.store(state.end, std::memory_order_relaxed);
}

void mvvm::MotorDriverStatusViewModel::onFault(domain::common::MotorFault fault) {
    fault_.store(fault, std::memory_order_relaxed);
}

