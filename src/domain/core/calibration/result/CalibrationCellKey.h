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

        bool operator==(const CalibrationCellKey& rhs) const {
            return point_id == rhs.point_id && source_id == rhs.source_id && direction == rhs.direction;
        }
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONCELLKEY_H