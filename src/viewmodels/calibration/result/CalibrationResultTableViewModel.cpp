#include "CalibrationResultTableViewModel.h"

#include "domain/ports/calibration/result/ICalibrationResultSource.h"

using namespace mvvm;
using namespace domain::common;

CalibrationResultTableViewModel::CalibrationResultTableViewModel(CalibrationResultTableViewModelDeps deps)
    : result_source_(deps.result_source)
{
    result_source_.addObserver(*this);
    current_result.set(result_source_.currentResult());
}

CalibrationResultTableViewModel::~CalibrationResultTableViewModel() {
    result_source_.removeObserver(*this);
}

void CalibrationResultTableViewModel::onCalibrationResultUpdated(const CalibrationResult &result) {
    current_result.set(result);
}
