#ifndef CLEANGRADUATOR_CALIBRATIONCELLKEY_H
#define CLEANGRADUATOR_CALIBRATIONCELLKEY_H
#include "domain/core/angle/AngleSourceId.h"
#include "domain/core/calibration/recording/PointId.h"
#include "domain/core/drivers/motor/MotorDirection.h"

namespace domain::common {
    struct CalibrationCellKey {
        MotorDirection direction;
        PointId point;
        AngleSourceId source;
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONCELLKEY_H