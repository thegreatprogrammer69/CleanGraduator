#ifndef CLEANGRADUATOR_CALIBRATIONRESULTTABLEVIEWMODEL_H
#define CLEANGRADUATOR_CALIBRATIONRESULTTABLEVIEWMODEL_H
#include "domain/ports/calibration/result/ICalibrationResultObserver.h"
#include "domain/ports/calibration/result/IClibrationResultObserver.h"
#include "domain/ports/calibration/result/IClibrationResultValidationSource.h"
#include "viewmodels/Observable.h"

namespace domain::ports {
    class ICalibrationResultSource;
}

namespace mvvm {
    struct CalibrationResultTableViewModelDeps {
        domain::ports::ICalibrationResultSource& result_source;
        domain::ports::IClibrationResultValidationSource& validation_source;
    };

    class CalibrationResultTableViewModel final
        : domain::ports::ICalibrationResultObserver
        , domain::ports::IClibrationResultObserver {
    public:
        explicit CalibrationResultTableViewModel(CalibrationResultTableViewModelDeps deps);
        ~CalibrationResultTableViewModel() override;

        void onCalibrationResultUpdated(const domain::common::CalibrationResult &result) override;
        void onClibrationResultUpdated(
            const domain::common::CalibrationResult& result,
            const domain::common::ClibrationResultValidation& validation) override;

        Observable<std::optional<domain::common::CalibrationResult>> current_result;
        Observable<std::optional<domain::common::ClibrationResultValidation>> current_validation;

    private:
        domain::ports::ICalibrationResultSource& result_source_;
        domain::ports::IClibrationResultValidationSource& validation_source_;
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONRESULTTABLEVIEWMODEL_H
