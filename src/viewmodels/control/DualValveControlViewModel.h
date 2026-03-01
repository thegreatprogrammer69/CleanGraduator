#ifndef CLEANGRADUATOR_VALVECONTROLVIEWMODEL_H
#define CLEANGRADUATOR_VALVECONTROLVIEWMODEL_H
#include "domain/core/drivers/motor/MotorFlapsState.h"
#include "domain/ports/drivers/motor/IMotorDriverObserver.h"
#include "viewmodels/Observable.h"

namespace domain::ports {
    struct IMotorDriver;
}

namespace mvvm {
    struct DualValveControlViewModelDeps {
        domain::ports::IMotorDriver& motor_driver;
    };

    class DualValveControlViewModel final : domain::ports::IMotorDriverObserver {
    public:
        explicit DualValveControlViewModel(DualValveControlViewModelDeps deps);
        ~DualValveControlViewModel() override;

        void openInputFlap();
        void openOutputFlap();
        void closeFlaps();

        void onMotorEvent(const domain::common::MotorDriverEvent &event) override;

        Observable<domain::common::MotorFlapsState> flaps_state{domain::common::MotorFlapsState::Uninitialized};

    private:
        domain::ports::IMotorDriver& motor_driver_;
    };
}


#endif //CLEANGRADUATOR_VALVECONTROLVIEWMODEL_H