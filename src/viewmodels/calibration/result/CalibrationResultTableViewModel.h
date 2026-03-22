#ifndef CLEANGRADUATOR_CALIBRATIONRESULTTABLEVIEWMODEL_H
#define CLEANGRADUATOR_CALIBRATIONRESULTTABLEVIEWMODEL_H
#include <unordered_map>

#include "domain/ports/calibration/recording/ICalibrationRecorderObserver.h"
#include "domain/ports/calibration/result/ICalibrationResultObserver.h"
#include "domain/ports/calibration/result/ICalibrationResultValidationObserver.h"
#include "viewmodels/Observable.h"

namespace domain::ports {
    class ICalibrationRecorder;
    class ICalibrationResultSource;
    class ICalibrationResultValidationSource;
}

namespace mvvm {
    struct CalibrationResultInfo final {
        std::unordered_map<domain::common::SourceId, float> current_angles;
        std::unordered_map<domain::common::SourceId, int> forward_measurement_counts;
        std::unordered_map<domain::common::SourceId, int> backward_measurement_counts;

        bool operator==(const CalibrationResultInfo& other) const {
            return current_angles == other.current_angles
                && forward_measurement_counts == other.forward_measurement_counts
                && backward_measurement_counts == other.backward_measurement_counts;
        }
    };

    struct CalibrationResultTableViewModelDeps {
        domain::ports::ICalibrationRecorder& recorder;
        domain::ports::ICalibrationResultSource& result_source;
        domain::ports::ICalibrationResultValidationSource& validation_source;
    };

    class CalibrationResultTableViewModel final
        : public domain::ports::ICalibrationRecorderObserver
        , public domain::ports::ICalibrationResultObserver
        , public domain::ports::ICalibrationResultValidationObserver {
    public:
        explicit CalibrationResultTableViewModel(CalibrationResultTableViewModelDeps deps);
        ~CalibrationResultTableViewModel() override;

        void onCalibrationRecorderEvent(const domain::common::CalibrationRecorderEvent& ev) override;
        void onCalibrationResultUpdated(const domain::common::CalibrationResult &result) override;
        void onCalibrationResultValidationUpdated(const domain::common::CalibrationResultValidation& validation) override;

        Observable<CalibrationResultInfo> info;
        Observable<std::optional<domain::common::CalibrationResult>> current_result;
        Observable<std::optional<domain::common::CalibrationResultValidation>> current_validation;

    private:
        void resetInfo();

    private:
        domain::ports::ICalibrationRecorder& recorder_;
        domain::ports::ICalibrationResultSource& result_source_;
        domain::ports::ICalibrationResultValidationSource& validation_source_;
        CalibrationResultInfo info_;
        std::optional<domain::common::CalibrationSessionId> active_session_id_;
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONRESULTTABLEVIEWMODEL_H
