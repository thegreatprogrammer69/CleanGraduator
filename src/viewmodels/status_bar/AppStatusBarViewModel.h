#ifndef CLEANGRADUATOR_APPSTATUSBARVIEWMODEL_H
#define CLEANGRADUATOR_APPSTATUSBARVIEWMODEL_H
#include "domain/core/measurement/Timestamp.h"
#include "domain/ports/calibration/lifecycle/ICalibrationLifecycleObserver.h"
#include "viewmodels/Observable.h"


namespace domain::ports {
    struct ICalibrationLifecycle;
    struct IClock;
}

namespace mvvm {
    struct AppStatusBarViewModelDeps {
        domain::ports::ICalibrationLifecycle& lifecycle;
        domain::ports::IClock& session_clock;
        domain::ports::IClock& uptime_clock;
    };
    class AppStatusBarViewModel final : public domain::ports::ICalibrationLifecycleObserver {
    public:
        explicit AppStatusBarViewModel(AppStatusBarViewModelDeps deps);
        ~AppStatusBarViewModel();

        domain::common::CalibrationLifecycleState state();
        domain::common::Timestamp sessionTime();
        domain::common::Timestamp uptimeTime();

    protected:
        void onCalibrationLifecycleStateChanged(domain::common::CalibrationLifecycleState newState) override;

    private:
        std::atomic<domain::common::CalibrationLifecycleState> current_state_{};

        domain::ports::ICalibrationLifecycle& lifecycle_;
        domain::ports::IClock& session_clock_;
        domain::ports::IClock& uptime_clock_;
    };
}

#endif //CLEANGRADUATOR_APPSTATUSBARVIEWMODEL_H