#ifndef CLEANGRADUATOR_CALIBRATIONRESULTVALIDATIONVIEWMODEL_H
#define CLEANGRADUATOR_CALIBRATIONRESULTVALIDATIONVIEWMODEL_H

#include <optional>

#include "application/orchestrators/settings/CalibrationContextProvider.h"
#include "domain/ports/calibration/result/ICalibrationResultObserver.h"
#include "domain/ports/calibration/result/IClibrationResultValidationSource.h"
#include "shared/ThreadSafeObserverList.h"
#include "viewmodels/Observable.h"

namespace domain::ports {
class ICalibrationResultSource;
}

namespace mvvm {
class InfoSettingsViewModel;
class CalibrationSessionControlViewModel;

struct CalibrationResultValidationViewModelDeps {
    domain::ports::ICalibrationResultSource& result_source;
    application::orchestrators::CalibrationContextProvider& context_provider;
    InfoSettingsViewModel& info_settings;
    CalibrationSessionControlViewModel& session_control;
};

class CalibrationResultValidationViewModel final
    : public domain::ports::ICalibrationResultObserver
    , public domain::ports::IClibrationResultValidationSource {
public:
    explicit CalibrationResultValidationViewModel(CalibrationResultValidationViewModelDeps deps);
    ~CalibrationResultValidationViewModel() override;

    void onCalibrationResultUpdated(const domain::common::CalibrationResult& result) override;

    const std::optional<domain::common::ClibrationResultValidation>& currentValidation() const override;
    void addObserver(domain::ports::IClibrationResultValidationObserver& observer) override;
    void removeObserver(domain::ports::IClibrationResultValidationObserver& observer) override;

private:
    void revalidate();
    domain::common::ClibrationResultValidation buildValidation(const domain::common::CalibrationResult& result) const;
    void validateSweepAngle(const domain::common::CalibrationResult& result,
                            domain::common::ClibrationResultValidation& validation) const;
    void validateHysteresis(const domain::common::CalibrationResult& result,
                            domain::common::ClibrationResultValidation& validation) const;
    float hysteresisLimit(const domain::common::CalibrationResult& result) const;
    void publish();

private:
    domain::ports::ICalibrationResultSource& result_source_;
    application::orchestrators::CalibrationContextProvider& context_provider_;
    InfoSettingsViewModel& info_settings_;
    CalibrationSessionControlViewModel& session_control_;
    std::optional<domain::common::CalibrationResult> current_result_;
    std::optional<domain::common::ClibrationResultValidation> current_validation_;
    ThreadSafeObserverList<domain::ports::IClibrationResultValidationObserver> observers_;
    Observable<int>::Subscription precision_sub_;
    Observable<bool>::Subscription ku_sub_;
};
}

#endif // CLEANGRADUATOR_CALIBRATIONRESULTVALIDATIONVIEWMODEL_H
