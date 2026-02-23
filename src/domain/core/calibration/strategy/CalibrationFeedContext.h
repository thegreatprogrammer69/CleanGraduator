#ifndef CLEANGRADUATOR_CALIBRATIONSTRATEGYFEEDCONTEXT_H
#define CLEANGRADUATOR_CALIBRATIONSTRATEGYFEEDCONTEXT_H
#include <optional>

#include "domain/core/calibration/common/CalibrationMode.h"
#include "domain/core/drivers/motor/MotorDirection.h"
#include "domain/core/drivers/motor/MotorLimitsState.h"
#include "domain/core/pressure/PressurePacket.h"

namespace domain::common {

    struct CalibrationFeedContext {
        float timestamp;
        float pressure;

        std::optional<int> current_point;
        CalibrationMode mode;

        MotorLimitsState limits;
    };

}

#endif //CLEANGRADUATOR_CALIBRATIONSTRATEGYFEEDCONTEXT_H