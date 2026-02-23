#ifndef CLEANGRADUATOR_CALIBRATIONBEGINCONTEXT_H
#define CLEANGRADUATOR_CALIBRATIONBEGINCONTEXT_H
#include "domain/core/calibration/common/CalibrationMode.h"
#include "domain/core/calibration/common/PressurePoints.h"

namespace domain::common {

    struct CalibrationBeginContext {
        PressureUnit pressure_unit;

        CalibrationMode calibration_mode;
        PressurePoints pressure_points;
    };

}

#endif //CLEANGRADUATOR_CALIBRATIONBEGINCONTEXT_H