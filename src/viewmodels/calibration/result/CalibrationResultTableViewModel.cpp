#include "CalibrationResultTableViewModel.h"

#include "domain/ports/calibration/result/ICalibrationResultSource.h"
#include "domain/ports/calibration/result/ICalibrationResultValidationSource.h"

using namespace mvvm;
using namespace domain::common;

CalibrationResultTableViewModel::CalibrationResultTableViewModel(CalibrationResultTableViewModelDeps deps)
    : result_source_(deps.result_source)
    , validation_source_(deps.validation_source)
    , calibration_series_(deps.calibration_series)
{
    result_source_.addObserver(*this);
    validation_source_.addObserver(*this);
    current_result.set(result_source_.currentResult());
    current_validation.set(validation_source_.currentValidation());

    source_ids_sub_ = calibration_series_.source_ids.subscribe([this](const auto&) {
        refreshInfoSnapshot();
    }, false);
    current_pressure_sub_ = calibration_series_.current_pressure.subscribe([this](const auto&) {
        refreshInfoSnapshot();
    }, false);
    current_angle_sub_ = calibration_series_.current_angle.subscribe([this](const auto&) {
        refreshInfoSnapshot();
    }, false);

    refreshInfoSnapshot();
}

CalibrationResultTableViewModel::~CalibrationResultTableViewModel() {
    validation_source_.removeObserver(*this);
    result_source_.removeObserver(*this);
}

void CalibrationResultTableViewModel::onCalibrationResultUpdated(const CalibrationResult &result) {
    current_result.set(result);
    refreshInfoSnapshot();
}

void CalibrationResultTableViewModel::onCalibrationResultValidationUpdated(const CalibrationResultValidation& validation) {
    current_validation.set(validation, true);
}

void CalibrationResultTableViewModel::refreshInfoSnapshot()
{
    InfoSnapshot snapshot;

    if (const auto result = result_source_.currentResult(); result.has_value()) {
        snapshot.source_ids = result->sources();
    } else {
        snapshot.source_ids = calibration_series_.openedSources();
    }

    for (const auto& source_id : snapshot.source_ids) {
        MeasurementCounts counts;
        for (const auto& entry : calibration_series_.angleHistory(source_id)) {
            if (entry.in_session) {
                ++counts.forward;
            } else {
                ++counts.backward;
            }
            snapshot.current_angle_by_source[source_id] = entry.angle;
        }
        snapshot.counts_by_source[source_id] = counts;
    }

    info_snapshot.set(std::move(snapshot), true);
}
