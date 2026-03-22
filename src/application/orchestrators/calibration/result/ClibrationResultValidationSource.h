#ifndef CLEANGRADUATOR_CLIBRATIONRESULTVALIDATIONSOURCE_H
#define CLEANGRADUATOR_CLIBRATIONRESULTVALIDATIONSOURCE_H

#include "application/orchestrators/settings/CalibrationContextProvider.h"
#include "domain/ports/calibration/result/IClibrationResultValidationSource.h"
#include "domain/ports/calibration/result/ICalibrationResultObserver.h"
#include "domain/ports/calibration/result/ICalibrationResultSource.h"
#include "shared/ThreadSafeObserverList.h"

namespace application::orchestrators {

struct ClibrationResultValidationSourceDeps {
    domain::ports::ICalibrationResultSource& result_source;
    CalibrationContextProvider& context_provider;
};

class ClibrationResultValidationSource final
    : public domain::ports::IClibrationResultValidationSource
    , public domain::ports::ICalibrationResultObserver {
public:
    explicit ClibrationResultValidationSource(ClibrationResultValidationSourceDeps deps);
    ~ClibrationResultValidationSource() override;

    const std::optional<domain::common::ClibrationResultValidation>& currentValidation() const override;
    void addObserver(domain::ports::IClibrationResultObserver& observer) override;
    void removeObserver(domain::ports::IClibrationResultObserver& observer) override;
    void revalidate() override;
    void setKuModeEnabled(bool enabled) override;
    bool kuModeEnabled() const override;

    void onCalibrationResultUpdated(const domain::common::CalibrationResult& result) override;

private:
    domain::common::ClibrationResultValidation buildValidation(const domain::common::CalibrationResult& result) const;
    void notifyObservers() const;

    domain::ports::ICalibrationResultSource& result_source_;
    CalibrationContextProvider& context_provider_;
    ThreadSafeObserverList<domain::ports::IClibrationResultObserver> observers_;
    std::optional<domain::common::CalibrationResult> current_result_;
    std::optional<domain::common::ClibrationResultValidation> current_validation_;
    bool ku_mode_enabled_{false};
};

} // namespace application::orchestrators

#endif // CLEANGRADUATOR_CLIBRATIONRESULTVALIDATIONSOURCE_H
