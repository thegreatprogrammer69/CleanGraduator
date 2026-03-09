#include "CalibrationResultBuilder.h"

const domain::common::CalibrationResult & CalibrationResultBuilder::currentResult() const {
    return calibration_result_;
}

void CalibrationResultBuilder::addObserver(domain::ports::ICalibrationResultObserver &o) {
    observers_.add(o);
}

void CalibrationResultBuilder::removeObserver(domain::ports::ICalibrationResultObserver &o) {
    observers_.remove(o);
}
