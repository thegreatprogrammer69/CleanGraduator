#ifndef CLEANGRADUATOR_APPSTATUSBARVIEWMODEL_H
#define CLEANGRADUATOR_APPSTATUSBARVIEWMODEL_H
#include "domain/core/measurement/Timestamp.h"
#include "domain/ports/lifecycle/IProcessLifecycleObserver.h"
#include "viewmodels/Observable.h"


namespace domain::ports {
    struct IClock;
    struct IProcessLifecycle;
}

namespace mvvm {
    struct AppStatusBarViewModelDeps {
        domain::ports::IProcessLifecycle& lifecycle;
        domain::ports::IClock& session_clock;
        domain::ports::IClock& uptime_clock;
    };
    class AppStatusBarViewModel final : public domain::ports::IProcessLifecycleObserver {
    public:
        explicit AppStatusBarViewModel(AppStatusBarViewModelDeps deps);
        ~AppStatusBarViewModel();

        domain::common::ProcessLifecycleState state();
        domain::common::Timestamp sessionTime();
        domain::common::Timestamp uptimeTime();

    protected:
        void onStateChanged(domain::common::ProcessLifecycleState new_state) override;

    private:
        std::atomic<domain::common::ProcessLifecycleState> current_state_{};

        domain::ports::IProcessLifecycle& lifecycle_;
        domain::ports::IClock& session_clock_;
        domain::ports::IClock& uptime_clock_;
    };
}

#endif //CLEANGRADUATOR_APPSTATUSBARVIEWMODEL_H