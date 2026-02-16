#ifndef CLEANGRADUATOR_PROCESSLIFECYCLE_H
#define CLEANGRADUATOR_PROCESSLIFECYCLE_H

#include <algorithm>
#include <mutex>
#include <vector>

#include "domain/core/process/ProcessLifecycleState.h"
#include "domain/ports/lifecycle/IProcessLifecycle.h"
#include "domain/ports/lifecycle/IProcessLifecycleObserver.h"
#include "infrastructure/clock/SessionClock.h"

namespace infra::process {

    class ProcessLifecycle final : public domain::ports::IProcessLifecycle {
    public:
        explicit ProcessLifecycle(
            domain::common::ProcessLifecycleState initial =
                domain::common::ProcessLifecycleState::Idle);

        ~ProcessLifecycle() override = default;

        // Если тебе нужен доступ к “сессионным” часам процесса
        domain::ports::IClock& clock();

        bool canStart() const override;
        bool canStop() const override;

        void markIdle() override;
        void markForward() override;
        void markBackward() override;
        void markStopping() override;
        void markEmergencyStopped() override;

        domain::common::ProcessLifecycleState state() const override;

        void addObserver(domain::ports::IProcessLifecycleObserver&) override;
        void removeObserver(domain::ports::IProcessLifecycleObserver&) override;

    private:
        void setState(domain::common::ProcessLifecycleState newState);
        void notify(domain::common::ProcessLifecycleState newState);

    private:
        clock::SessionClock clock_;

        mutable std::mutex state_mutex_;
        domain::common::ProcessLifecycleState state_;

        mutable std::mutex observers_mutex_;
        std::vector<domain::ports::IProcessLifecycleObserver*> observers_;
    };

} // namespace infra::process

#endif // CLEANGRADUATOR_PROCESSLIFECYCLE_H
