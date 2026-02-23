#ifndef CLEANGRADUATOR_CALIBRATIONSESSIONCONTROLVIEWMODEL_H
#define CLEANGRADUATOR_CALIBRATIONSESSIONCONTROLVIEWMODEL_H

#include "application/orchestrators/calibration/session/CalibrationSessionController.h"
#include "domain/ports/calibration/lifecycle/ICalibrationLifecycleObserver.h"
#include "viewmodels/Observable.h"

namespace mvvm {

struct CalibrationSessionControlViewModelDeps {
    application::orchestrators::CalibrationSessionController& controller;
};

class CalibrationSessionControlViewModel final
    : public domain::ports::ICalibrationLifecycleObserver
{
public:
    explicit CalibrationSessionControlViewModel(CalibrationSessionControlViewModelDeps deps);
    ~CalibrationSessionControlViewModel() override;

    void setCalibrationMode(domain::common::CalibrationMode mode);
    domain::common::CalibrationMode calibrationMode() const;

    void start();
    void stop();
    void emergencyStop();

    Observable<std::string> error_text{""};
    Observable<bool> is_running{false};

private:
    void onCalibrationLifecycleStateChanged(
        domain::common::CalibrationLifecycleState newState,
        const std::string& lastError) override;

private:
    application::orchestrators::CalibrationSessionController& controller_;
    domain::common::CalibrationMode calibration_mode_{domain::common::CalibrationMode::Full};
};

}

#endif //CLEANGRADUATOR_CALIBRATIONSESSIONCONTROLVIEWMODEL_H
