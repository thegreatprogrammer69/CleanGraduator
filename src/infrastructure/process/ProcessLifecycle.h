#ifndef CLEANGRADUATOR_PROCESSLIFECYCLE_H
#define CLEANGRADUATOR_PROCESSLIFECYCLE_H

#include <vector>
#include <algorithm>

#include "domain/core/process/ProcessLifecycleState.h"
#include "domain/ports/inbound/IProcessLifecycle.h"
#include "domain/ports/outbound/IProcessLifecycleObserver.h"
#include "infrastructure/clock/SessionClock.h"


namespace infra::process {

class ProcessLifecycle final : public domain::ports::IProcessLifecycle {
    using State = domain::common::ProcessLifecycleState;
    using Observer = domain::ports::IProcessLifecycleObserver;
public:
    explicit ProcessLifecycle(
        State initial = State::Idle);

    domain::ports::IClock& clock();

    bool canStart() const override;
    bool canStop() const override;

    void markIdle() override;
    void markForward() override;
    void markBackward() override;
    void markStopping() override;

    State state() const override;
    void subscribe(Observer& observer) override;

private:
    void setState(State newState);
    void notify() const;

private:
    clock::SessionClock clock_;
    State state_;
    std::vector<Observer*> observers_;
};

}

#endif // CLEANGRADUATOR_PROCESSLIFECYCLE_H
