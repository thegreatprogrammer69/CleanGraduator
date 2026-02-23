#include "MotorDriverStatusViewModel.h"

#include "domain/core/drivers/motor/MotorError.h"
#include "../../domain/ports/drivers/motor/IMotorDriver.h"

using namespace domain::common;

mvvm::MotorDriverStatusViewModel::MotorDriverStatusViewModel(MotorDriverStatusViewModelDeps deps)
    : motor_driver_(deps.motor_driver)
{
    motor_driver_.addObserver(*this);
    is_running_.set(motor_driver_.isRunning());
    limits_state_.set(motor_driver_.limits());
    direction_.set(motor_driver_.direction());
}

mvvm::MotorDriverStatusViewModel::~MotorDriverStatusViewModel() {
    motor_driver_.removeObserver(*this);
}

int mvvm::MotorDriverStatusViewModel::frequency() const {
    return motor_driver_.frequency();
}

void mvvm::MotorDriverStatusViewModel::onMotorStarted() {
    is_running_.set(true);
}

void mvvm::MotorDriverStatusViewModel::onMotorStopped() {
    is_running_.set(false);
}

void mvvm::MotorDriverStatusViewModel::onMotorStartFailed(const MotorError &error) {
    is_running_.set(false);
    error_.set(error.reason);
}

void mvvm::MotorDriverStatusViewModel::onMotorLimitsStateChanged(MotorLimitsState state) {
    limits_state_.set(state);
}

void mvvm::MotorDriverStatusViewModel::onMotorDirectionChanged(MotorDirection direction) {
    direction_.set(direction);
}

