#include "MotorControlInteractor.h"

#include "IMotorControlObserver.h"
#include "domain/ports/motor/IMotorDriver.h"

#include <algorithm>

using namespace application::orchestrators;

MotorControlInteractor::MotorControlInteractor(domain::ports::IMotorDriver &driver): driver_(driver) {
    driver_.addObserver(*this);
}

MotorControlInteractor::~MotorControlInteractor() {
    driver_.removeObserver(*this);
}

void MotorControlInteractor::start(const int frequency, const domain::common::MotorDirection dir) {
    if (driver_.isRunning()) return;
    driver_.setDirection(dir);
    driver_.setFrequency(frequency);
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

void MotorControlInteractor::onStarted() {
    notifyRunning(true);
}

void MotorControlInteractor::onStopped() {
    notifyRunning(false);
}

void MotorControlInteractor::onLimitsStateChanged(domain::common::MotorLimitsState) {
}

void MotorControlInteractor::onDirectionChanged(domain::common::MotorDirection) {
}

void MotorControlInteractor::onFault(const domain::common::MotorFault &) {
    notifyRunning(false);
}

void MotorControlInteractor::notifyRunning(bool running) {
    for (auto* obs : observers_)
        obs->onRunningChanged(running);
}
