#ifndef CLEANGRADUATOR_CALIBRATIONRESULTTABLEVIEWMODEL_H
#define CLEANGRADUATOR_CALIBRATIONRESULTTABLEVIEWMODEL_H
#include "domain/ports/calibration/result/ICalibrationResultObserver.h"
#include "viewmodels/Observable.h"

namespace domain::ports {
    class ICalibrationResultSource;
}

namespace mvvm {
    struct CalibrationResultTableViewModelDeps {
        domain::ports::ICalibrationResultSource& result_source;
    };

    class CalibrationResultTableViewModel final : domain::ports::ICalibrationResultObserver {
    public:
        explicit CalibrationResultTableViewModel(CalibrationResultTableViewModelDeps deps);
        ~CalibrationResultTableViewModel() override;

        void onCalibrationResultUpdated(const domain::common::CalibrationResult &result) override;

        Observable<std::optional<domain::common::CalibrationResult>> current_result;

    private:
        domain::ports::ICalibrationResultSource& result_source_;
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONRESULTTABLEVIEWMODEL_H