#ifndef CLEANGRADUATOR_CALIBRATIONSESSIONCONTROLVIEWMODEL_H
#define CLEANGRADUATOR_CALIBRATIONSESSIONCONTROLVIEWMODEL_H

#include <string>

#include "application/orchestrators/calibration/process/CalibrationOrchestratorInput.h"
#include "application/orchestrators/calibration/process/CalibrationOrchestratorState.h"
#include "application/usecases/calibration/CalibrationSessionControl.h"
#include "application/ports/settings/IInfoSettingsStorage.h"
#include "domain/ports/calibration/result/ICalibrationResultValidationSource.h"
#include "domain/core/calibration/common/CalibrationMode.h"
#include "viewmodels/Observable.h"

namespace mvvm {

struct CalibrationSessionControlViewModelDeps {
    application::usecase::CalibrationSessionControl& control;
    application::ports::IInfoSettingsStorage& settings_storage;
    domain::ports::ICalibrationResultValidationSource& validation_source;
};

class CalibrationSessionControlViewModel final : public application::ports::CalibrationOrchestratorObserver
{
public:
    explicit CalibrationSessionControlViewModel(CalibrationSessionControlViewModelDeps deps);
    ~CalibrationSessionControlViewModel() override;

    void setCalibrationMode(domain::common::CalibrationMode mode);
    void startCalibration();
    void setKuModeEnabled(bool enabled);
    void setCenteredMarkEnabled(bool enabled);
    void stopCalibration();
    void emergencyStop();

    void onCalibrationOrchestratorEvent(const application::orchestrators::CalibrationOrchestratorEvent &ev) override;


    Observable<domain::common::CalibrationMode> selected_mode{domain::common::CalibrationMode::Full};
    Observable<std::string> error_text{std::string()};
    Observable<bool> ku_mode_enabled{false};
    Observable<bool> centered_mark_enabled{false};
    Observable<bool> can_start{true};
    Observable<bool> can_stop{false};
    Observable<bool> can_abort{false};

private:
    void applyState(application::orchestrators::CalibrationOrchestratorState state, const std::string& last_error);

    application::usecase::CalibrationSessionControl& control_;
    application::ports::IInfoSettingsStorage& settings_storage_;
    domain::ports::ICalibrationResultValidationSource& validation_source_;
};

} // namespace mvvm

#endif // CLEANGRADUATOR_CALIBRATIONSESSIONCONTROLVIEWMODEL_H
