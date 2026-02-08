#include "ProcessLifecycle.h"


namespace infra::process {

ProcessLifecycle::ProcessLifecycle(const State initial): state_(initial) {}

domain::ports::IClock & ProcessLifecycle::clock() {
    return clock_;
}

bool ProcessLifecycle::canStart() const {
    return state_ == State::Idle;
}

bool ProcessLifecycle::canStop() const {
    return state_ == State::Forward ||
           state_ == State::Backward;
}

void ProcessLifecycle::markIdle() {
    setState(State::Idle);
}

void ProcessLifecycle::markForward() {
    if (canStart()) {
        clock_.start();
        setState(State::Forward);
    }
}

void ProcessLifecycle::markBackward() {
    if (canStart()) {
        setState(State::Backward);
    }
}

void ProcessLifecycle::markStopping() {
    if (canStop()) {
        clock_.stop();
        setState(State::Stopping);
    }
}

ProcessLifecycle::State ProcessLifecycle::state() const {
    return state_;
}

void ProcessLifecycle::subscribe(Observer &observer) {
    observers_.push_back(&observer);
}

void ProcessLifecycle::setState(const State newState) {
    if (state_ == newState) {
        return;
    }
    state_ = newState;
    notify();
}

void ProcessLifecycle::notify() const {
    for (auto* observer : observers_) {
        observer->onStateChanged(state_);
    }
}

}
