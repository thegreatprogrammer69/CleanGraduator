#ifndef CLEANGRADUATOR_CALIBRATIONINPUT_H
#define CLEANGRADUATOR_CALIBRATIONINPUT_H
#include "domain/core/calibration/calculation/CalibrationMode.h"
#include "domain/core/calibration/calculation/GaugePressurePoints.h"
#include "domain/core/calibration/recording/CalibrationDataset.h"

namespace domain::common {

    struct CalibrationCalculatorInput {
        CalibrationMode mode;
        GaugePressurePoints points;
        CalibrationDataset dataset;
    };

}



#endif //CLEANGRADUATOR_CALIBRATIONINPUT_H