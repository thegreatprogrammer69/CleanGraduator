#ifndef CLEANGRADUATOR_CLIBRATIONRESULTVALIDATIONSOURCE_H
#define CLEANGRADUATOR_CLIBRATIONRESULTVALIDATIONSOURCE_H

#include <optional>

#include "domain/ports/calibration/result/IClibrationResultValidationSource.h"
#include "domain/ports/calibration/result/ICalibrationResultObserver.h"
#include "domain/ports/calibration/result/ICalibrationResultSource.h"
#include "shared/ThreadSafeObserverList.h"
#include "viewmodels/Observable.h"

namespace mvvm {

class InfoSettingsViewModel;
class CalibrationSessionControlViewModel;

struct ClibrationResultValidationSourceDeps {
    domain::ports::ICalibrationResultSource& result_source;
    InfoSettingsViewModel& info_settings;
    CalibrationSessionControlViewModel& calibration_control;
};

class ClibrationResultValidationSource final
    : public domain::ports::ICalibrationResultObserver
    , public domain::ports::IClibrationResultValidationSource {
public:
    explicit ClibrationResultValidationSource(ClibrationResultValidationSourceDeps deps);
    ~ClibrationResultValidationSource() override;

    void onCalibrationResultUpdated(const domain::common::CalibrationResult& result) override;

    const std::optional<domain::common::ClibrationResultValidation>& currentValidation() const override;
    void addObserver(domain::ports::IClibrationResultObserver& observer) override;
    void removeObserver(domain::ports::IClibrationResultObserver& observer) override;

private:
    void revalidate();
    void notifyObservers() const;
    double hysteresisLimitPercent() const;
    bool isKuEnabled() const;

    ClibrationResultValidationSourceDeps deps_;
    std::optional<domain::common::CalibrationResult> current_result_;
    std::optional<domain::common::ClibrationResultValidation> current_validation_;
    ThreadSafeObserverList<domain::ports::IClibrationResultObserver> observers_;
    mvvm::Observable<int>::Subscription precision_sub_;
    mvvm::Observable<bool>::Subscription ku_sub_;
};

} // namespace mvvm

#endif // CLEANGRADUATOR_CLIBRATIONRESULTVALIDATIONSOURCE_H
