#include "DualValve.h"
#include "domain/ports/motor/IDualValveDriver.h"

using namespace domain::common;

DualValve::DualValve(ports::IDualValveDriver &driver): driver_(driver) {}

ValvePosition DualValve::position() const noexcept {
    return state_;
}

bool DualValve::isClosed() const noexcept {
    return state_ == ValvePosition::Closed;
}

bool DualValve::isIntakeOpen() const noexcept {
    return state_ == ValvePosition::IntakeOpen;
}

bool DualValve::isExhaustOpen() const noexcept {
    return state_ == ValvePosition::ExhaustOpen;
}

void DualValve::openIntake() {
    if (state_ == ValvePosition::IntakeOpen)
        return; // идемпотентность

    driver_.openInputFlap();
    state_ = ValvePosition::IntakeOpen;
}

void DualValve::openExhaust() {
    if (state_ == ValvePosition::ExhaustOpen)
        return;

    driver_.openOutputFlap();
    state_ = ValvePosition::ExhaustOpen;
}

void DualValve::close() {
    if (state_ == ValvePosition::Closed)
        return;

    driver_.closeFlaps();
    state_ = ValvePosition::Closed;
}
