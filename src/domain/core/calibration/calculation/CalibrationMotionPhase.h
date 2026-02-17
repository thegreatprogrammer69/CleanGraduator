#ifndef CLEANGRADUATOR_CALIBRATIONMOTIONSTATE_H
#define CLEANGRADUATOR_CALIBRATIONMOTIONSTATE_H
#include "domain/core/motor/motor/MotorDirection.h"

namespace domain::common {
    struct CalibrationMotionPhase {
        MotorDirection direction;
        int point_index;
    };
}
#endif //CLEANGRADUATOR_CALIBRATIONMOTIONSTATE_H