#ifndef CLEANGRADUATOR_CALIBRATIONRESULTTABLEVIEWMODEL_H
#define CLEANGRADUATOR_CALIBRATIONRESULTTABLEVIEWMODEL_H

#include "domain/ports/calibration/result/ICalibrationResultObserver.h"
#include "domain/ports/calibration/result/ICalibrationResultValidationObserver.h"
#include "viewmodels/Observable.h"
#include "viewmodels/calibration/recording/CalibrationSeriesViewModel.h"

namespace domain::ports {
    class ICalibrationResultSource;
    class ICalibrationResultValidationSource;
}

namespace mvvm {
    struct CalibrationResultTableViewModelDeps {
        domain::ports::ICalibrationResultSource& result_source;
        domain::ports::ICalibrationResultValidationSource& validation_source;
        CalibrationSeriesViewModel& calibration_series;
    };

    class CalibrationResultTableViewModel final : public domain::ports::ICalibrationResultObserver, public domain::ports::ICalibrationResultValidationObserver {
    public:
        explicit CalibrationResultTableViewModel(CalibrationResultTableViewModelDeps deps);
        ~CalibrationResultTableViewModel() override;

        void onCalibrationResultUpdated(const domain::common::CalibrationResult &result) override;
        void onCalibrationResultValidationUpdated(const domain::common::CalibrationResultValidation& validation) override;

        Observable<std::optional<domain::common::CalibrationResult>> current_result;
        Observable<std::optional<domain::common::CalibrationResultValidation>> current_validation;
        Observable<int> supplemental_revision;

        int angleMeasurementCount(domain::common::SourceId source_id, domain::common::MotorDirection direction) const;
        std::optional<float> currentAngle(domain::common::SourceId source_id) const;

    private:
        domain::ports::ICalibrationResultSource& result_source_;
        domain::ports::ICalibrationResultValidationSource& validation_source_;
        CalibrationSeriesViewModel& calibration_series_;
        mvvm::Observable<std::pair<domain::common::SourceId, CalibrationSeriesViewModel::AngleEntry>>::Subscription current_angle_sub_;
        mvvm::Observable<int>::Subscription series_revision_sub_;
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONRESULTTABLEVIEWMODEL_H
