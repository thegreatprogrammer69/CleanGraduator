#include "MotorControlInteractor.h"
#include "domain/ports/drivers/motor/IMotorDriver.h"

using namespace application::orchestrators;

MotorControlInteractor::MotorControlInteractor(domain::ports::IMotorDriver &driver): driver_(driver) {

}

MotorControlInteractor::~MotorControlInteractor() {

}

void MotorControlInteractor::start(const int frequency, const domain::common::MotorDirection dir) {
    driver_.setDirection(dir);
    driver_.setFrequency(domain::common::MotorFrequency(frequency));
    driver_.start();
}

void MotorControlInteractor::stop() {
    driver_.stop();
}

