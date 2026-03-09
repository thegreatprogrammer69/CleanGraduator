#ifndef CLEANGRADUATOR_CALIBRATIONINPUT_H
#define CLEANGRADUATOR_CALIBRATIONINPUT_H
#include "../common/CalibrationMode.h"
#include "../common/PressurePoints.h"
#include "domain/core/calibration/recording/CalibrationSession.h"

namespace domain::common {

    struct CalibrationCalculatorInput {
        float pressure_point;
        AngleSeries angle_series;
        PressureSeries pressure_series;
    };

}



#endif //CLEANGRADUATOR_CALIBRATIONINPUT_H