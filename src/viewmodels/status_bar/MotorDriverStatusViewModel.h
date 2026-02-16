#ifndef CLEANGRADUATOR_MOTORDRIVERSTATUSVIEWMODEL_H
#define CLEANGRADUATOR_MOTORDRIVERSTATUSVIEWMODEL_H

#include <atomic>

#include "domain/core/motor/motor/MotorDirection.h"
#include "domain/core/motor/motor/MotorFault.h"
#include "domain/core/motor/motor/MotorLimitsState.h"
#include "domain/ports/motor/IMotorDriverObserver.h"

namespace domain::ports {
    struct IMotorDriver;
}

namespace mvvm {

    struct MotorDriverStatusViewModelDeps {
        domain::ports::IMotorDriver& motor_driver;
    };

    class MotorDriverStatusViewModel final : public domain::ports::IMotorDriverObserver {
    public:
        explicit MotorDriverStatusViewModel(MotorDriverStatusViewModelDeps deps);
        ~MotorDriverStatusViewModel() override;

        bool isRunning() const;
        int frequencyHz() const;
        domain::common::MotorDirection direction() const;
        domain::common::MotorLimitsState limits() const;
        domain::common::MotorFault fault() const;

    protected:
        void onStarted() override;
        void onStopped() override;
        void onFrequencyChanged(int hz) override;
        void onDirectionChanged(domain::common::MotorDirection dir) override;
        void onLimitsChanged(domain::common::MotorLimitsState state) override;
        void onFault(domain::common::MotorFault fault) override;

    private:
        std::atomic_bool is_running_{false};
        std::atomic_int frequency_hz_{0};
        std::atomic<domain::common::MotorDirection> direction_{domain::common::MotorDirection::Forward};
        std::atomic_bool home_limit_{false};
        std::atomic_bool end_limit_{false};
        std::atomic<domain::common::MotorFault> fault_{domain::common::MotorFault::None};

        domain::ports::IMotorDriver& motor_driver_;
    };

}

#endif //CLEANGRADUATOR_MOTORDRIVERSTATUSVIEWMODEL_H

