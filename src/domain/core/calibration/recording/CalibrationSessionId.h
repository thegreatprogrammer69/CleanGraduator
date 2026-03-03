#ifndef CLEANGRADUATOR_CALIBRATIONSESSIONID_H
#define CLEANGRADUATOR_CALIBRATIONSESSIONID_H
#include "PointId.h"
#include "domain/core/drivers/motor/MotorDirection.h"

namespace domain::common {
    struct CalibrationSessionId {
        PointId point;
        MotorDirection direction;
        bool operator==(const CalibrationSessionId& rhs) const { return point == rhs.point && direction == rhs.direction; }
    };
}

namespace std {

    template<>
    struct hash<domain::common::PointId> {
        size_t operator()(const domain::common::PointId& p) const noexcept {
            return std::hash<unsigned int>{}(p.id);
        }
    };

    template<>
    struct hash<domain::common::CalibrationSessionId> {
        size_t operator()(const domain::common::CalibrationSessionId& s) const noexcept {
            size_t h1 = std::hash<domain::common::PointId>{}(s.point);
            size_t h2 = std::hash<int>{}(static_cast<int>(s.direction));

            // корректный hash combine (boost-style)
            return h1 ^ (h2 + 0x9e3779b97f4a7c15ULL + (h1 << 6) + (h1 >> 2));
        }
    };

}

#endif //CLEANGRADUATOR_CALIBRATIONSESSIONID_H