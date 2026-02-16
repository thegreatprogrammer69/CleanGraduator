#include "MotorDriverStatusViewModel.h"

#include "domain/ports/motor/IMotorDriver.h"

mvvm::MotorDriverStatusViewModel::MotorDriverStatusViewModel(MotorDriverStatusViewModelDeps deps)
    : motor_driver_(deps.motor_driver)
{
    motor_driver_.addObserver(*this);
    fault.set(motor_driver_.fault());
}

mvvm::MotorDriverStatusViewModel::~MotorDriverStatusViewModel() {
    motor_driver_.removeObserver(*this);
}

bool mvvm::MotorDriverStatusViewModel::isRunning() const {
    return is_running_.load(std::memory_order_relaxed);
}

int mvvm::MotorDriverStatusViewModel::frequencyHz() const {
    return motor_driver_.frequency();
}

domain::common::MotorDirection mvvm::MotorDriverStatusViewModel::direction() const {
    return motor_driver_.direction();
}

domain::common::MotorLimitsState mvvm::MotorDriverStatusViewModel::limits() const {
    return motor_driver_.limits();
}

void mvvm::MotorDriverStatusViewModel::onStarted() {
    is_running_.store(true, std::memory_order_relaxed);
}

void mvvm::MotorDriverStatusViewModel::onStopped() {
    is_running_.store(false, std::memory_order_relaxed);
}

void mvvm::MotorDriverStatusViewModel::onFault(const domain::common::MotorFault& fault) {
    this->fault.set(fault);
}

