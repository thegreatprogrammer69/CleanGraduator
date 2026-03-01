#include "MotorDriverStatusViewModel.h"

#include <type_traits>
#include <variant>

#include "domain/core/drivers/motor/MotorDriverState.h"
#include "domain/ports/drivers/motor/IMotorDriver.h"

using namespace domain::common;

mvvm::MotorDriverStatusViewModel::MotorDriverStatusViewModel(MotorDriverStatusViewModelDeps deps)
    : motor_driver_(deps.motor_driver) {
    motor_driver_.addObserver(*this);
    is_running_.set(motor_driver_.state() == MotorDriverState::Running);
    limits_state_.set(motor_driver_.limits());
    direction_.set(motor_driver_.direction());
}

mvvm::MotorDriverStatusViewModel::~MotorDriverStatusViewModel() {
    motor_driver_.removeObserver(*this);
}

int mvvm::MotorDriverStatusViewModel::frequency() const {
    return motor_driver_.frequency().value;
}

void mvvm::MotorDriverStatusViewModel::onMotorEvent(const MotorDriverEvent& event) {
    std::visit(
        [this](const auto& e) {
            using T = std::decay_t<decltype(e)>;
            if constexpr (std::is_same_v<T, MotorDriverEvent::Started>) {
                is_running_.set(true);
                error_.set("");
            } else if constexpr (std::is_same_v<T, MotorDriverEvent::Stopped>
                                 || std::is_same_v<T, MotorDriverEvent::StoppedAtHome>
                                 || std::is_same_v<T, MotorDriverEvent::StoppedAtEnd>) {
                is_running_.set(false);
            } else if constexpr (std::is_same_v<T, MotorDriverEvent::Fault>) {
                is_running_.set(false);
                error_.set(e.error.reason);
            } else if constexpr (std::is_same_v<T, MotorDriverEvent::LimitsChanged>) {
                limits_state_.set(e.limits);
            } else if constexpr (std::is_same_v<T, MotorDriverEvent::DirectionChanged>) {
                direction_.set(e.direction);
            }
        },
        event.data);
}
