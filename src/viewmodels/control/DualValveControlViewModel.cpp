//
// Created by mint on 18.02.2026.
//

#include "DualValveControlViewModel.h"

#include "domain/core/drivers/motor/MotorDriverEvent.h"
#include "domain/ports/drivers/motor/IMotorDriver.h"

mvvm::DualValveControlViewModel::DualValveControlViewModel(DualValveControlViewModelDeps deps)
    : motor_driver_(deps.motor_driver)
{
    motor_driver_.addObserver(*this);
    flaps_state.set(motor_driver_.flapsState());
}

mvvm::DualValveControlViewModel::~DualValveControlViewModel() {
    motor_driver_.removeObserver(*this);
}

void mvvm::DualValveControlViewModel::openInputFlap() {
    motor_driver_.setFlapsState(domain::common::MotorFlapsState::IntakeOpened);
    flaps_state.set(motor_driver_.flapsState());
}

void mvvm::DualValveControlViewModel::openOutputFlap() {
    motor_driver_.setFlapsState(domain::common::MotorFlapsState::ExhaustOpened);
    flaps_state.set(motor_driver_.flapsState());
}

void mvvm::DualValveControlViewModel::closeFlaps() {
    motor_driver_.setFlapsState(domain::common::MotorFlapsState::FlapsClosed);
}

void mvvm::DualValveControlViewModel::onMotorEvent(const domain::common::MotorDriverEvent &ev) {
    auto data = ev.data;
    std::visit([this](auto& e)
    {
        using T = std::decay_t<decltype(e)>;

        if constexpr (std::is_same_v<T, domain::common::MotorDriverEvent::FlapsStateChanged>) {
            flaps_state.set(e.state);
        } else {
            // На случай, если состояние клапанов изменилось вне явного FlapsStateChanged,
            // синхронизируем VM снимком из драйвера.
            flaps_state.set(motor_driver_.flapsState());
        }


    }, data);
}
