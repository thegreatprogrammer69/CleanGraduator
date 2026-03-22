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

    current_angle_sub_ = calibration_series_.current_angle.subscribe([this](const auto&) {
        supplemental_revision.set(supplemental_revision.get_copy() + 1, true);
    }, false);
    series_revision_sub_ = calibration_series_.revision.subscribe([this](const auto& e) {
        supplemental_revision.set(e.new_value, true);
    }, false);
}

CalibrationResultTableViewModel::~CalibrationResultTableViewModel() {
    validation_source_.removeObserver(*this);
    result_source_.removeObserver(*this);
}

void CalibrationResultTableViewModel::onCalibrationResultUpdated(const CalibrationResult &result) {
    current_result.set(result);
}

void CalibrationResultTableViewModel::onCalibrationResultValidationUpdated(const CalibrationResultValidation& validation) {
    current_validation.set(validation, true);
}

int CalibrationResultTableViewModel::angleMeasurementCount(SourceId source_id, MotorDirection direction) const {
    return calibration_series_.angleMeasurementCount(source_id, direction);
}

std::optional<float> CalibrationResultTableViewModel::currentAngle(SourceId source_id) const {
    return calibration_series_.currentAngle(source_id);
}
