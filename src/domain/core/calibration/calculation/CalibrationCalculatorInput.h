#ifndef CLEANGRADUATOR_CALIBRATIONINPUT_H
#define CLEANGRADUATOR_CALIBRATIONINPUT_H
#include "../common/CalibrationMode.h"
#include "../common/PressurePoints.h"

namespace domain::common {

    struct CalibrationCalculatorInput {
        CalibrationMode mode;
        PressurePoints points;
    };

}



#endif //CLEANGRADUATOR_CALIBRATIONINPUT_H