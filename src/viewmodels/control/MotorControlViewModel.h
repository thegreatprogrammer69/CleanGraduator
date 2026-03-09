#ifndef CLEANGRADUATOR_MOTORCONTROLVIEWMODEL_H
#define CLEANGRADUATOR_MOTORCONTROLVIEWMODEL_H
#include "domain/core/drivers/motor/MotorDirection.h"
#include "domain/ports/drivers/motor/IMotorDriverObserver.h"
#include "viewmodels/Observable.h"

namespace application::orchestrators {
    class MotorControlInteractor;
}

namespace domain::ports {
    struct IMotorDriver;
}

namespace mvvm {
    struct MotorControlViewModelDeps {
        application::orchestrators::MotorControlInteractor& interactor;
        domain::ports::IMotorDriver& motor;
    };

    class MotorControlViewModel final : domain::ports::IMotorDriverObserver
    {
    public:
        explicit MotorControlViewModel(MotorControlViewModelDeps deps);
        ~MotorControlViewModel();

        void start();
        void stop();

        void setFrequency(int f);
        void setDirection(domain::common::MotorDirection d);

        Observable<bool> is_running{false};

    private:
        // ===== IMotorDriverObserver =====
        void onMotorEvent(const domain::common::MotorDriverEvent &event) override;

    private:
        application::orchestrators::MotorControlInteractor& interactor_;
        domain::ports::IMotorDriver& motor_;
        int frequency_ = 0;
        domain::common::MotorDirection direction_ = domain::common::MotorDirection::Forward;
    };
}


#endif //CLEANGRADUATOR_MOTORCONTROLVIEWMODEL_H