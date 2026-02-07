#include "ProcessLifecycle.h"


namespace infra::process {

ProcessLifecycle::ProcessLifecycle(ProcessLifecycleState initial): state_(initial) {}

IClock & ProcessLifecycle::clock() {
    return clock_;
}

bool ProcessLifecycle::canStart() const {
    return state_ == ProcessLifecycleState::Idle;
}

bool ProcessLifecycle::canStop() const {
    return state_ == ProcessLifecycleState::Forward ||
           state_ == ProcessLifecycleState::Backward;
}

void ProcessLifecycle::markIdle() {
    setState(ProcessLifecycleState::Idle);
}

void ProcessLifecycle::markForward() {
    if (canStart()) {
        clock_.start();
        setState(ProcessLifecycleState::Forward);
    }
}

void ProcessLifecycle::markBackward() {
    if (canStart()) {
        setState(ProcessLifecycleState::Backward);
    }
}

void ProcessLifecycle::markStopping() {
    if (canStop()) {
        clock_.stop();
        setState(ProcessLifecycleState::Stopping);
    }
}

ProcessLifecycleState ProcessLifecycle::state() const {
    return state_;
}

void ProcessLifecycle::subscribe(IProcessLifecycleObserver &observer) {
    observers_.push_back(&observer);
}

void ProcessLifecycle::setState(ProcessLifecycleState newState) {
    if (state_ == newState) {
        return;
    }
    state_ = newState;
    notify();
}

void ProcessLifecycle::notify() {
    for (auto* observer : observers_) {
        observer->onStateChanged(state_);
    }
}

}
