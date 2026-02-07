#ifndef CLEANGRADUATOR_PROCESSLIFECYCLE_H
#define CLEANGRADUATOR_PROCESSLIFECYCLE_H

#include <vector>
#include <algorithm>

#include "../../domain/core/process/ProcessLifecycleState.h"
#include "domain/ports/inbound/IProcessLifecycle.h"
#include "domain/ports/outbound/IProcessLifecycleObserver.h"
#include "infrastructure/clock/SessionClock.h"


namespace infra::process {

class ProcessLifecycle final : public IProcessLifecycle {
public:
    explicit ProcessLifecycle(
        ProcessLifecycleState initial = ProcessLifecycleState::Idle);

    IClock& clock();

    bool canStart() const override;
    bool canStop() const override;

    void markIdle() override;
    void markForward() override;
    void markBackward() override;
    void markStopping() override;

    ProcessLifecycleState state() const override;
    void subscribe(IProcessLifecycleObserver& observer) override;

private:
    void setState(ProcessLifecycleState newState);
    void notify();

private:
    SessionClock clock_;
    ProcessLifecycleState state_;
    std::vector<IProcessLifecycleObserver*> observers_;
};

}

#endif // CLEANGRADUATOR_PROCESSLIFECYCLE_H
