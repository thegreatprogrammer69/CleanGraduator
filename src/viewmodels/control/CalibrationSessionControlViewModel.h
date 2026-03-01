#ifndef CLEANGRADUATOR_CALIBRATIONSESSIONCONTROLVIEWMODEL_H
#define CLEANGRADUATOR_CALIBRATIONSESSIONCONTROLVIEWMODEL_H

#include <string>

#include "application/orchestrators/calibration/process/CalibrationOrchestrator.h"
#include "application/orchestrators/calibration/process/CalibrationOrchestratorState.h"
#include "domain/core/calibration/common/CalibrationMode.h"
#include "viewmodels/Observable.h"

namespace mvvm {

struct CalibrationSessionControlViewModelDeps {

};

class CalibrationSessionControlViewModel final : public application::ports::CalibrationOrchestratorObserver
{
public:
    explicit CalibrationSessionControlViewModel(CalibrationSessionControlViewModelDeps deps);
    ~CalibrationSessionControlViewModel() override;

    void setCalibrationMode(domain::common::CalibrationMode mode);
    void startCalibration();
    void stopCalibration();
    void emergencyStop();

    void onCalibrationOrchestratorEvent(const application::orchestrators::CalibrationOrchestratorEvent &ev) override;


    Observable<domain::common::CalibrationMode> selected_mode{domain::common::CalibrationMode::Full};
    Observable<std::string> error_text{std::string()};
    Observable<bool> can_start{true};
    Observable<bool> can_stop{false};
    Observable<bool> can_abort{false};

private:
    // application::orchestrators::CalibrationSessionController& session_controller_;
    // domain::ports::ICalibrationLifecycle& lifecycle_;
};

} // namespace mvvm

#endif // CLEANGRADUATOR_CALIBRATIONSESSIONCONTROLVIEWMODEL_H
