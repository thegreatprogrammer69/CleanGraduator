#ifndef CLEANGRADUATOR_MOTORCONTROLVIEWMODEL_H
#define CLEANGRADUATOR_MOTORCONTROLVIEWMODEL_H
#include "application/orchestrators/motor/IMotorControlObserver.h"
#include "domain/core/motor/motor/MotorDirection.h"
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
    };

    class MotorControlViewModel final
        : application::orchestrators::IMotorControlObserver
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
        // ===== IMotorControlObserver =====

        void onRunningChanged(bool running) override;

    private:
        application::orchestrators::MotorControlInteractor& interactor_;
        int frequency_ = 0;
        domain::common::MotorDirection direction_ = domain::common::MotorDirection::Forward;
    };
}


#endif //CLEANGRADUATOR_MOTORCONTROLVIEWMODEL_H