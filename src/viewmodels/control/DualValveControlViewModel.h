#ifndef CLEANGRADUATOR_VALVECONTROLVIEWMODEL_H
#define CLEANGRADUATOR_VALVECONTROLVIEWMODEL_H
#include "domain/ports/motor/IValveDriverObserver.h"
#include "viewmodels/Observable.h"

namespace domain::ports {
    struct IValveDriver;
}

namespace mvvm {
    struct DualValveControlViewModelDeps {
        domain::ports::IValveDriver& dual_valve_driver;
    };

    class DualValveControlViewModel final : domain::ports::IValveDriverObserver {
    public:
        explicit DualValveControlViewModel(DualValveControlViewModelDeps deps);
        ~DualValveControlViewModel() override;

        void openInputFlap();
        void openOutputFlap();
        void closeFlaps();

        void onInputFlapOpened() override;
        void onOutputFlapOpened() override;
        void onFlapsClosed() override;

        enum FlapsState {
            FlapsUninitialized, InputFlapOpened, OutputFlapOpened, FlapsClosed
        };

        Observable<FlapsState> flaps_state{FlapsUninitialized};

    private:
        domain::ports::IValveDriver& dual_valve_driver_;
    };
}


#endif //CLEANGRADUATOR_VALVECONTROLVIEWMODEL_H