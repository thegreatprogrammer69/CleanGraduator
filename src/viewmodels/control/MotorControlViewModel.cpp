#include "MotorControlViewModel.h"

#include <variant>

#include "application/orchestrators/motor/MotorControlInteractor.h"
#include "domain/core/drivers/motor/MotorDriverEvent.h"
#include "domain/ports/drivers/motor/IMotorDriver.h"

mvvm::MotorControlViewModel::MotorControlViewModel(MotorControlViewModelDeps deps)
    : interactor_(deps.interactor)
    , motor_(deps.motor) {
    motor_.addObserver(*this);
}

mvvm::MotorControlViewModel::~MotorControlViewModel() {
    motor_.removeObserver(*this);
}

void mvvm::MotorControlViewModel::start() {
    interactor_.start(frequency_, direction_);
}

void mvvm::MotorControlViewModel::stop() {
    interactor_.stop();
}

void mvvm::MotorControlViewModel::runForwardWithMaxFrequency() {
    interactor_.start(motor_.frequencyLimits().maxHz, domain::common::MotorDirection::Forward);
}

void mvvm::MotorControlViewModel::runBackwardWithMaxFrequency() {
    interactor_.start(motor_.frequencyLimits().maxHz, domain::common::MotorDirection::Backward);
}

void mvvm::MotorControlViewModel::setFrequency(int f) {
    frequency_ = f;
}

void mvvm::MotorControlViewModel::setDirection(domain::common::MotorDirection d) {
    direction_ = d;
}

void mvvm::MotorControlViewModel::onMotorEvent(const domain::common::MotorDriverEvent &ev) {
    std::visit([this](const auto& event) {
        using T = std::decay_t<decltype(event)>;

        if constexpr (std::is_same_v<T, domain::common::MotorDriverEvent::Started>) {
            is_running.set(true);
        } else if constexpr (std::is_same_v<T, domain::common::MotorDriverEvent::Stopped>) {
            is_running.set(false);
        }

    }, ev.data);
}
