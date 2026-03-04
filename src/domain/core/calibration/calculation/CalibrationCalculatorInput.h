#ifndef CLEANGRADUATOR_CALIBRATIONINPUT_H
#define CLEANGRADUATOR_CALIBRATIONINPUT_H
#include "../common/CalibrationMode.h"
#include "../common/PressurePoints.h"
#include "domain/core/calibration/recording/CalibrationSession.h"

namespace domain::common {

    struct CalibrationCalculatorInput {
        CalibrationSession calibration_session;
    };

}



#endif //CLEANGRADUATOR_CALIBRATIONINPUT_H