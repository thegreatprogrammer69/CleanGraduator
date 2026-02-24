#ifndef CLEANGRADUATOR_MOTORDRIVERSTATUSVIEWMODEL_H
#define CLEANGRADUATOR_MOTORDRIVERSTATUSVIEWMODEL_H

#include <atomic>

#include "domain/core/drivers/motor/MotorDirection.h"
#include "domain/core/drivers/motor/MotorLimitsState.h"
#include "../../domain/ports/drivers/motor/IMotorDriverObserver.h"
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

        Observable<bool> is_running_;
        Observable<std::string> error_;
        Observable<domain::common::MotorDirection> direction_;
        Observable<domain::common::MotorLimitsState> limits_state_;

        int frequency() const;

    protected:
        void onMotorStarted() override;
        void onMotorStopped() override;
        void onMotorStartFailed(const domain::common::MotorDriverError &) override;
        void onMotorLimitsStateChanged(domain::common::MotorLimitsState) override;
        void onMotorDirectionChanged(domain::common::MotorDirection) override;

    private:
        domain::ports::IMotorDriver& motor_driver_;
    };

}

#endif //CLEANGRADUATOR_MOTORDRIVERSTATUSVIEWMODEL_H

