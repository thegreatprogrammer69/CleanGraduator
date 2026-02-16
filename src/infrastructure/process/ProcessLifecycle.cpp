#include "ProcessLifecycle.h"

namespace infra::process {

using domain::common::ProcessLifecycleState;

ProcessLifecycle::ProcessLifecycle(ProcessLifecycleState initial)
    : state_(initial)
{
}

domain::ports::IClock& ProcessLifecycle::clock()
{
    return clock_;
}

ProcessLifecycleState ProcessLifecycle::state() const
{
    std::lock_guard lock(state_mutex_);
    return state_;
}

bool ProcessLifecycle::canStart() const
{
    const auto s = state();
    // Базовая политика: стартуем только из Idle.
    // EmergencyStopped требует явного выхода (обычно отдельным reset'ом),
    // но ты пока такого метода не добавлял в порт — поэтому start запрещён.
    return s == ProcessLifecycleState::Idle;
}

bool ProcessLifecycle::canStop() const
{
    const auto s = state();
    // Стоп уместен, когда реально “едем”.
    return s == ProcessLifecycleState::Forward ||
           s == ProcessLifecycleState::Backward;
}

void ProcessLifecycle::markIdle()
{
    setState(ProcessLifecycleState::Idle);
}

void ProcessLifecycle::markForward()
{
    setState(ProcessLifecycleState::Forward);
}

void ProcessLifecycle::markBackward()
{
    setState(ProcessLifecycleState::Backward);
}

void ProcessLifecycle::markStopping()
{
    setState(ProcessLifecycleState::Stopping);
}

void ProcessLifecycle::markEmergencyStopped()
{
    // Emergency — оверрайд: может прилететь из любого состояния.
    setState(ProcessLifecycleState::EmergencyStopped);
}

void ProcessLifecycle::addObserver(domain::ports::IProcessLifecycleObserver& obs)
{
    std::lock_guard lock(observers_mutex_);

    auto it = std::find(observers_.begin(), observers_.end(), &obs);
    if (it == observers_.end()) {
        observers_.push_back(&obs);
    }
}

void ProcessLifecycle::removeObserver(domain::ports::IProcessLifecycleObserver& obs)
{
    std::lock_guard lock(observers_mutex_);

    auto it = std::remove(observers_.begin(), observers_.end(), &obs);
    observers_.erase(it, observers_.end());
}

void ProcessLifecycle::setState(ProcessLifecycleState newState)
{
    ProcessLifecycleState oldState;

    {
        std::lock_guard lock(state_mutex_);
        oldState = state_;
        if (oldState == newState)
            return;

        state_ = newState;
    }

    notify(newState);
}

void ProcessLifecycle::notify(ProcessLifecycleState newState)
{
    std::vector<domain::ports::IProcessLifecycleObserver*> snapshot;
    {
        std::lock_guard lock(observers_mutex_);
        snapshot = observers_;
    }

    for (auto* o : snapshot) {
        if (o) {
            o->onStateChanged(newState);
        }
    }
}

} // namespace infra::process
