#include "CalibrationResultTableViewModel.h"

#include "domain/ports/calibration/result/ICalibrationResultSource.h"
#include "domain/ports/calibration/result/IClibrationResultValidationSource.h"

using namespace mvvm;
using namespace domain::common;

CalibrationResultTableViewModel::CalibrationResultTableViewModel(CalibrationResultTableViewModelDeps deps)
    : result_source_(deps.result_source)
    , validation_source_(deps.validation_source)
{
    result_source_.addObserver(*this);
    validation_source_.addObserver(*this);
    current_result.set(result_source_.currentResult());
    current_validation.set(validation_source_.currentValidation());
}

CalibrationResultTableViewModel::~CalibrationResultTableViewModel() {
    validation_source_.removeObserver(*this);
    result_source_.removeObserver(*this);
}

void CalibrationResultTableViewModel::onCalibrationResultUpdated(const CalibrationResult &result) {
    current_result.set(result);
}

void CalibrationResultTableViewModel::onClibrationResultValidationUpdated(const ClibrationResultValidation& validation) {
    current_validation.set(validation);
}
