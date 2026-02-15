#include "../DualValve.h"
#include "domain/ports/motor/IDualValveDriver.h"

domain::common::DualValve::DualValve(ports::IDualValveDriver &driver): driver_(driver) {}

ValvePosition domain::common::DualValve::position() const noexcept {
    return state_;
}

bool domain::common::DualValve::isClosed() const noexcept {
    return state_ == ValvePosition::Closed;
}

bool domain::common::DualValve::isIntakeOpen() const noexcept {
    return state_ == ValvePosition::IntakeOpen;
}

bool domain::common::DualValve::isExhaustOpen() const noexcept {
    return state_ == ValvePosition::ExhaustOpen;
}

void domain::common::DualValve::openIntake() {
    if (state_ == ValvePosition::IntakeOpen)
        return; // идемпотентность

    driver_.moveToIntake();
    state_ = ValvePosition::IntakeOpen;
}

void domain::common::DualValve::openExhaust() {
    if (state_ == ValvePosition::ExhaustOpen)
        return;

    driver_.moveToExhaust();
    state_ = ValvePosition::ExhaustOpen;
}

void domain::common::DualValve::close() {
    if (state_ == ValvePosition::Closed)
        return;

    driver_.moveToClosed();
    state_ = ValvePosition::Closed;
}
