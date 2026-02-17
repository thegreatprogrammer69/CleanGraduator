#ifndef CLEANGRADUATOR_CALIBRATIONINPUT_H
#define CLEANGRADUATOR_CALIBRATIONINPUT_H
#include "domain/core/calibration/calculation/CalibrationMotionMode.h"
#include "domain/core/calibration/calculation/PressurePoints.h"
#include "domain/core/calibration/recording/CalibrationDataset.h"

namespace domain::common {

    struct CalibrationCalculatorInput {
        CalibrationMotionMode mode;
        PressurePoints points;
        CalibrationDataset dataset;
    };

}



#endif //CLEANGRADUATOR_CALIBRATIONINPUT_H