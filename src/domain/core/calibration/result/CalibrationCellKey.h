#ifndef CLEANGRADUATOR_CALIBRATIONCELLKEY_H
#define CLEANGRADUATOR_CALIBRATIONCELLKEY_H
#include "domain/core/angle/SourceId.h"
#include "domain/core/calibration/recording/PointId.h"
#include "domain/core/drivers/motor/MotorDirection.h"

namespace domain::common {
    struct CalibrationCellKey {
        PointId point_id;
        SourceId source_id;
        MotorDirection direction;
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONCELLKEY_H