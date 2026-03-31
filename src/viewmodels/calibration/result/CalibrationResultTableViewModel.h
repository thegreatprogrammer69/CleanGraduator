#ifndef CLEANGRADUATOR_CALIBRATIONRESULTTABLEVIEWMODEL_H
#define CLEANGRADUATOR_CALIBRATIONRESULTTABLEVIEWMODEL_H

#include <map>
#include <unordered_map>

#include "domain/core/drivers/motor/MotorDirection.h"
#include "application/ports/settings/IInfoSettingsStorage.h"
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
        std::unordered_map<domain::common::SourceId, float> total_angles;
        std::unordered_map<domain::common::SourceId,
            std::map<domain::common::MotorDirection, float>> nonlinearities;
        std::unordered_map<domain::common::SourceId,
            std::map<domain::common::MotorDirection, int>> measurement_counts;
        std::unordered_map<domain::common::SourceId, float> current_angles;
        std::unordered_map<domain::common::SourceId,
            std::map<domain::common::MotorDirection, float>> center_deviations_deg;
        float max_center_deviation_deg{0.9F};
        bool centered_mark_enabled{false};

        bool operator==(const CalibrationResultInfo& other) const {
            return total_angles == other.total_angles
                && nonlinearities == other.nonlinearities
                && measurement_counts == other.measurement_counts
                && current_angles == other.current_angles
                && center_deviations_deg == other.center_deviations_deg
                && max_center_deviation_deg == other.max_center_deviation_deg
                && centered_mark_enabled == other.centered_mark_enabled;
        }
    };

    struct CalibrationResultTableViewModelDeps {
        domain::ports::ICalibrationResultSource& result_source;
        domain::ports::ICalibrationResultValidationSource& validation_source;
        domain::ports::ICalibrationRecorder& recorder;
        application::ports::IInfoSettingsStorage& settings_storage;
    };

    class CalibrationResultTableViewModel final
        : public domain::ports::ICalibrationResultObserver
        , public domain::ports::ICalibrationResultValidationObserver
        , public domain::ports::ICalibrationRecorderObserver {
    public:
        explicit CalibrationResultTableViewModel(CalibrationResultTableViewModelDeps deps);
        ~CalibrationResultTableViewModel() override;

        void onCalibrationResultUpdated(const domain::common::CalibrationResult &result) override;
        void onCalibrationResultValidationUpdated(const domain::common::CalibrationResultValidation& validation) override;
        void onCalibrationRecorderEvent(const domain::common::CalibrationRecorderEvent& ev) override;

        Observable<std::optional<domain::common::CalibrationResult>> current_result;
        Observable<std::optional<domain::common::CalibrationResultValidation>> current_validation;
        Observable<CalibrationResultInfo> current_info;

    private:
        void resetInfo();
        void updateInfoFromResult(const domain::common::CalibrationResult& result);
        void updateCurrentInfo();
        void refreshMeasurementCountsFromRecorder();
        static std::optional<float> calculateNonlinearity(
            const domain::common::CalibrationResult& result,
            domain::common::SourceId source_id,
            domain::common::MotorDirection direction);
        static std::optional<float> calculateCenterDeviationDeg(
            const domain::common::CalibrationResult& result,
            domain::common::SourceId source_id,
            domain::common::MotorDirection direction);
        void refreshSettings();

    private:
        domain::ports::ICalibrationResultSource& result_source_;
        domain::ports::ICalibrationResultValidationSource& validation_source_;
        domain::ports::ICalibrationRecorder& recorder_;
        application::ports::IInfoSettingsStorage& settings_storage_;
        CalibrationResultInfo info_;
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONRESULTTABLEVIEWMODEL_H
