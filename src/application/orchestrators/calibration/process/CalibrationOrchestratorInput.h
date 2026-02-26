#ifndef CLEANGRADUATOR_CALIBRATIONPROCESSORCHESTRATORINPUT_H
#define CLEANGRADUATOR_CALIBRATIONPROCESSORCHESTRATORINPUT_H
#include "domain/core/calibration/common/CalibrationMode.h"
#include "domain/core/calibration/common/PressurePoints.h"
#include "domain/core/measurement/AngleUnit.h"
#include "domain/core/measurement/PressureUnit.h"
#include <vector>


namespace application::orchestrators {
    struct CalibrationOrchestratorInput {
        domain::common::CalibrationMode calibration_mode;
        domain::common::PressureUnit pressure_unit;
        domain::common::AngleUnit angle_unit;
        domain::common::PressurePoints pressure_points;
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONPROCESSORCHESTRATORINPUT_H