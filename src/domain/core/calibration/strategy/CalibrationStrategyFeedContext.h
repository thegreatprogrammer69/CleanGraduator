#ifndef CLEANGRADUATOR_CALIBRATIONSTRATEGYFEEDCONTEXT_H
#define CLEANGRADUATOR_CALIBRATIONSTRATEGYFEEDCONTEXT_H

#include "domain/core/drivers/motor/MotorLimitsState.h"

namespace domain::common {

    struct CalibrationStrategyFeedContext {
        float timestamp;
        float pressure;
        MotorLimitsState limits_state;
    };

}

#endif //CLEANGRADUATOR_CALIBRATIONSTRATEGYFEEDCONTEXT_H