#include "MotorControlViewModel.h"

#include "application/orchestrators/motor/MotorControlInteractor.h"

mvvm::MotorControlViewModel::MotorControlViewModel(MotorControlViewModelDeps deps)
    : interactor_(deps.interactor) {
    interactor_.addObserver(*this);
}

mvvm::MotorControlViewModel::~MotorControlViewModel() {
    interactor_.removeObserver(*this);
}

void mvvm::MotorControlViewModel::start() {
    interactor_.start(frequency_, direction_);
}

void mvvm::MotorControlViewModel::stop() {
    interactor_.stop();
}

void mvvm::MotorControlViewModel::setFrequency(int f) {
    frequency_ = f;
}

void mvvm::MotorControlViewModel::setDirection(domain::common::MotorDirection d) {
    direction_ = d;
}

void mvvm::MotorControlViewModel::onRunningChanged(bool running) {
    is_running.set(running);
}
