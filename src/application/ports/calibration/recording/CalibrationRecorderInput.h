#ifndef CLEANGRADUATOR_CALIBRATIONRECORDERINPUT_H
#define CLEANGRADUATOR_CALIBRATIONRECORDERINPUT_H
#include <vector>
#include "domain/core/angle/AngleSourceId.h"
#include "domain/core/measurement/AngleUnit.h"
#include "domain/core/measurement/PressureUnit.h"

namespace application::ports {
    struct CalibrationRecorderInput {
        std::vector<domain::common::AngleSourceId> active_sources_ids;
        domain::common::PressureUnit pressure_unit;
        domain::common::AngleUnit angle_unit;
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONRECORDERINPUT_H