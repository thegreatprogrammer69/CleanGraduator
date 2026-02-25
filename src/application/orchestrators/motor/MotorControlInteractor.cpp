#include "MotorControlInteractor.h"

#include "IMotorControlObserver.h"
#include "domain/ports/drivers/motor/IMotorDriver.h"

#include <algorithm>

#include "domain/core/drivers/motor/MotorDriverEvent.h"

using namespace application::orchestrators;

MotorControlInteractor::MotorControlInteractor(domain::ports::IMotorDriver &driver): driver_(driver) {
    driver_.addObserver(*this);
}

MotorControlInteractor::~MotorControlInteractor() {
    driver_.removeObserver(*this);
}

void MotorControlInteractor::start(const int frequency, const domain::common::MotorDirection dir) {
    driver_.setDirection(dir);
    driver_.setFrequency(domain::common::MotorFrequency(frequency));
    driver_.start();
}

void MotorControlInteractor::stop() {
    driver_.stop();
}


void MotorControlInteractor::addObserver(IMotorControlObserver &obs) {
    observers_.push_back(&obs);
}

void MotorControlInteractor::removeObserver(IMotorControlObserver& obs) {
    observers_.erase(
        std::remove(observers_.begin(), observers_.end(), &obs),
        observers_.end()
    );
}

void MotorControlInteractor::onMotorEvent(const domain::common::MotorDriverEvent &event) {
    using namespace domain::common;
    std::visit([this](const auto& e)
    {
        using T = std::decay_t<decltype(e)>;

        if constexpr (std::is_same_v<T, MotorDriverEvent::Started>) {
            notifyRunning(true);
        }

        else if constexpr (std::is_same_v<T, MotorDriverEvent::Stopped>) {
            notifyRunning(false);
        }

        else if constexpr (std::is_same_v<T, MotorDriverEvent::Fault>) {
            notifyRunning(false);
        }

    }, event.data);
}


void MotorControlInteractor::notifyRunning(const bool running) const {
    for (auto* obs : observers_)
        obs->onRunningChanged(running);
}
