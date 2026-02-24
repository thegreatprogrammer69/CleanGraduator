#ifndef CLEANGRADUATOR_CALIBRATIONSESSIONCONTROLVIEWMODEL_H
#define CLEANGRADUATOR_CALIBRATIONSESSIONCONTROLVIEWMODEL_H

#include <string>

#include "application/orchestrators/calibration/session/CalibrationSessionController.h"
#include "domain/ports/calibration/lifecycle/ICalibrationLifecycle.h"
#include "domain/ports/calibration/lifecycle/ICalibrationLifecycleObserver.h"
#include "viewmodels/Observable.h"

namespace mvvm {

struct CalibrationSessionControlViewModelDeps {
    application::orchestrators::CalibrationSessionController& session_controller;
    domain::ports::ICalibrationLifecycle& lifecycle;
};

class CalibrationSessionControlViewModel final
    : public domain::ports::ICalibrationLifecycleObserver
{
public:
    explicit CalibrationSessionControlViewModel(CalibrationSessionControlViewModelDeps deps);
    ~CalibrationSessionControlViewModel() override;

    void setCalibrationMode(domain::common::CalibrationMode mode);
    void startCalibration();
    void stopCalibration();
    void emergencyStop();

    void onCalibrationLifecycleStateChanged(
        domain::common::CalibrationLifecycleState newState,
        const std::string& lastError) override;

    Observable<domain::common::CalibrationMode> selected_mode{domain::common::CalibrationMode::Full};
    Observable<std::string> error_text{std::string()};
    Observable<bool> can_start{true};
    Observable<bool> can_stop{false};
    Observable<bool> can_abort{false};

private:
    application::orchestrators::CalibrationSessionController& session_controller_;
    domain::ports::ICalibrationLifecycle& lifecycle_;
};

} // namespace mvvm

#endif // CLEANGRADUATOR_CALIBRATIONSESSIONCONTROLVIEWMODEL_H
