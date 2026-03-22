#ifndef CLEANGRADUATOR_CALIBRATIONRESULTTABLEVIEWMODEL_H
#define CLEANGRADUATOR_CALIBRATIONRESULTTABLEVIEWMODEL_H
#include <unordered_map>
#include <vector>

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
        mvvm::CalibrationSeriesViewModel& calibration_series;
    };

    class CalibrationResultTableViewModel final : public domain::ports::ICalibrationResultObserver, public domain::ports::ICalibrationResultValidationObserver {
    public:
        struct MeasurementCounts final {
            int forward = 0;
            int backward = 0;

            bool operator==(const MeasurementCounts& other) const {
                return forward == other.forward && backward == other.backward;
            }
        };

        struct InfoSnapshot final {
            std::unordered_map<domain::common::SourceId, MeasurementCounts> counts_by_source{};
            std::unordered_map<domain::common::SourceId, float> current_angle_by_source{};
            std::vector<domain::common::SourceId> source_ids{};

            bool operator==(const InfoSnapshot& other) const {
                return counts_by_source == other.counts_by_source
                    && current_angle_by_source == other.current_angle_by_source
                    && source_ids == other.source_ids;
            }
        };

    public:
        explicit CalibrationResultTableViewModel(CalibrationResultTableViewModelDeps deps);
        ~CalibrationResultTableViewModel() override;

        void onCalibrationResultUpdated(const domain::common::CalibrationResult &result) override;
        void onCalibrationResultValidationUpdated(const domain::common::CalibrationResultValidation& validation) override;

        Observable<std::optional<domain::common::CalibrationResult>> current_result;
        Observable<std::optional<domain::common::CalibrationResultValidation>> current_validation;
        Observable<InfoSnapshot> info_snapshot;

    private:
        void refreshInfoSnapshot();

    private:
        domain::ports::ICalibrationResultSource& result_source_;
        domain::ports::ICalibrationResultValidationSource& validation_source_;
        mvvm::CalibrationSeriesViewModel& calibration_series_;
        mvvm::Observable<std::vector<domain::common::SourceId>>::Subscription source_ids_sub_;
        mvvm::Observable<mvvm::CalibrationSeriesViewModel::PressureEntry>::Subscription current_pressure_sub_;
        mvvm::Observable<std::pair<domain::common::SourceId, mvvm::CalibrationSeriesViewModel::AngleEntry>>::Subscription current_angle_sub_;
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONRESULTTABLEVIEWMODEL_H