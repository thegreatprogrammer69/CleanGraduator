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
#include "viewmodels/control/ICalibrationSoundNotifier.h"

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
    void aimCalibration();
    void setKuModeEnabled(bool enabled);
    void setCenteredMarkEnabled(bool enabled);
    void setReverseModeEnabled(bool enabled);
    void setSlowdownEnabled(bool enabled);
    void setPlayValveEnabled(bool enabled);
    void stopCalibration();
    void emergencyStop();

    void onCalibrationOrchestratorEvent(const application::orchestrators::CalibrationOrchestratorEvent &ev) override;
    void setSoundNotifier(ICalibrationSoundNotifier* notifier);

    Observable<domain::common::CalibrationMode> selected_mode{domain::common::CalibrationMode::Full};
    Observable<std::string> error_text{std::string()};
    Observable<bool> ku_mode_enabled{false};
    Observable<bool> centered_mark_enabled{false};
    Observable<bool> reverse_mode_enabled{false};
    Observable<bool> slowdown_enabled{false};
    Observable<bool> play_valve_enabled{false};
    Observable<std::string> status_text{std::string("Ожидание запуска")};
    Observable<bool> can_start{true};
    Observable<bool> can_stop{false};
    Observable<bool> can_abort{false};

private:
    void applyState(application::orchestrators::CalibrationOrchestratorState state, const std::string& last_error);

    application::usecase::CalibrationSessionControl& control_;
    application::ports::IInfoSettingsStorage& settings_storage_;
    domain::ports::ICalibrationResultValidationSource& validation_source_;
    ICalibrationSoundNotifier* sound_notifier_{nullptr};
    bool forward_phase_reported_{false};
};

} // namespace mvvm

#endif // CLEANGRADUATOR_CALIBRATIONSESSIONCONTROLVIEWMODEL_H
