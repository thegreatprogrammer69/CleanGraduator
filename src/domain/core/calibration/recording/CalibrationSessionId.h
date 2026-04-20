#ifndef CLEANGRADUATOR_CALIBRATIONSESSIONID_H
#define CLEANGRADUATOR_CALIBRATIONSESSIONID_H
#include "PointId.h"
#include "domain/core/drivers/motor/MotorDirection.h"

namespace domain::common {
    struct CalibrationSessionId {
        PointId point;
        MotorDirection direction;
        bool operator==(const CalibrationSessionId& rhs) const { return point == rhs.point && direction == rhs.direction; }
        bool operator<(const CalibrationSessionId& rhs) const { return point.id < rhs.point.id && direction < rhs.direction; }
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONSESSIONID_H