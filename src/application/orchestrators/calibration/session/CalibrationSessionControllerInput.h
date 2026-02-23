#ifndef CLEANGRADUATOR_CALIBRATIONSESSIONCONTROLLERINPUT_H
#define CLEANGRADUATOR_CALIBRATIONSESSIONCONTROLLERINPUT_H
#include "domain/core/calibration/common/CalibrationMode.h"

namespace application::orchestrators {
    struct CalibrationSessionControllerInput {
        domain::common::CalibrationMode calibration_mode;
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONSESSIONCONTROLLERINPUT_H