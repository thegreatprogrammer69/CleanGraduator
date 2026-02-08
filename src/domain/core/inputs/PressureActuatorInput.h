#ifndef CLEANGRADUATOR_PRESSUREACTUATORINPUT_H
#define CLEANGRADUATOR_PRESSUREACTUATORINPUT_H
#include "domain/core/calibration/CalibrationMode.h"
#include "domain/core/calibration/CalibrationPoints.h"

namespace domain::common {
    struct PressureActuatorInput {
        CalibrationPoints points;
        CalibrationMode mode;
    };
}

#endif //CLEANGRADUATOR_PRESSUREACTUATORINPUT_H