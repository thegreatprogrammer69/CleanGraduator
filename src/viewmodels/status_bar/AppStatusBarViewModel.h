#ifndef CLEANGRADUATOR_APPSTATUSBARVIEWMODEL_H
#define CLEANGRADUATOR_APPSTATUSBARVIEWMODEL_H

#include <atomic>

#include "application/orchestrators/calibration/process/CalibrationOrchestratorState.h"
#include "application/ports/calibration/orchestration/CalibrationOrchestratorObserver.h"
#include "domain/core/measurement/Timestamp.h"

namespace application::orchestrators {
class CalibrationOrchestrator;
}

namespace domain::ports {
struct IClock;
}

namespace mvvm {

struct AppStatusBarViewModelDeps {
    application::orchestrators::CalibrationOrchestrator& orchestrator;
    domain::ports::IClock& session_clock;
    domain::ports::IClock& uptime_clock;
};

class AppStatusBarViewModel final : public application::ports::CalibrationOrchestratorObserver {
public:
    explicit AppStatusBarViewModel(AppStatusBarViewModelDeps deps);
    ~AppStatusBarViewModel();

    application::orchestrators::CalibrationOrchestratorState state();
    domain::common::Timestamp sessionTime();
    domain::common::Timestamp uptimeTime();

    void onCalibrationOrchestratorEvent(const application::orchestrators::CalibrationOrchestratorEvent& ev) override;

private:
    std::atomic<application::orchestrators::CalibrationOrchestratorState> current_state_{};

    application::orchestrators::CalibrationOrchestrator& orchestrator_;
    domain::ports::IClock& session_clock_;
    domain::ports::IClock& uptime_clock_;
};

} // namespace mvvm

#endif //CLEANGRADUATOR_APPSTATUSBARVIEWMODEL_H
