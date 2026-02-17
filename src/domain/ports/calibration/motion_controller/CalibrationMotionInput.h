#ifndef CLEANGRADUATOR_CALIBRATIONMOTIONINPUT_H
#define CLEANGRADUATOR_CALIBRATIONMOTIONINPUT_H
#include "domain/core/calibration/calculation/CalibrationMotionMode.h"
#include "domain/core/calibration/calculation/PressurePoints.h"

namespace domain::ports {
    struct CalibrationMotionInput {
        common::CalibrationMotionMode mode;
        common::PressurePoints pressure_points;
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONMOTIONINPUT_H