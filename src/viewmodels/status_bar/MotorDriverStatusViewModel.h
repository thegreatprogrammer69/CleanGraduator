#ifndef CLEANGRADUATOR_MOTORDRIVERSTATUSVIEWMODEL_H
#define CLEANGRADUATOR_MOTORDRIVERSTATUSVIEWMODEL_H

#include <atomic>

#include "domain/core/motor/motor/MotorDirection.h"
#include "domain/core/motor/motor/MotorFault.h"
#include "domain/core/motor/motor/MotorLimitsState.h"
#include "domain/ports/motor/IMotorDriverObserver.h"
#include "viewmodels/Observable.h"

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

        Observable<domain::common::MotorFault> fault{};

    protected:
        // IMotorDriverObserver
        void onStarted() override;
        void onStopped() override;
        void onLimitsStateChanged(domain::common::MotorLimitsState) override;
        void onDirectionChanged(domain::common::MotorDirection) override;
        void onFault(const domain::common::MotorFault& fault) override;

    private:
        // В UI 3 состояния - остановлен и движется вперёд/назад
        std::atomic_bool is_running_{false};
        domain::ports::IMotorDriver& motor_driver_;
    };

}

#endif //CLEANGRADUATOR_MOTORDRIVERSTATUSVIEWMODEL_H

