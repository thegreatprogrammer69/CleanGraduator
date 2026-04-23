#ifndef CLEANGRADUATOR_CALIBRATIONBEGINCONTEXT_H
#define CLEANGRADUATOR_CALIBRATIONBEGINCONTEXT_H
#include "domain/core/calibration/common/CalibrationMode.h"
#include "domain/core/calibration/common/PressurePoints.h"

namespace domain::common {

    struct CalibrationStrategyBeginContext {
        CalibrationMode calibration_mode;
        PressurePoints pressure_points;
        PressureUnit pressure_unit;
        bool slowdown_at_checkpoints{false};
        bool play_valve{false};
    };

}

#endif //CLEANGRADUATOR_CALIBRATIONBEGINCONTEXT_H
