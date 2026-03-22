#ifndef CLEANGRADUATOR_CALIBRATIONCELLKEY_H
#define CLEANGRADUATOR_CALIBRATIONCELLKEY_H
#include "domain/core/angle/SourceId.h"
#include "domain/core/calibration/recording/PointId.h"
#include <functional>
#include "domain/core/drivers/motor/MotorDirection.h"

namespace domain::common {
    struct CalibrationCellKey {
        PointId point_id;
        SourceId source_id;
        MotorDirection direction;

        bool operator==(const CalibrationCellKey& other) const {
            return point_id == other.point_id && source_id == other.source_id && direction == other.direction;
        }
    };
}

namespace std {
    template<>
    struct hash<domain::common::CalibrationCellKey>
    {
        size_t operator()(const domain::common::CalibrationCellKey& key) const noexcept
        {
            size_t seed = std::hash<domain::common::PointId>{}(key.point_id);
            seed ^= std::hash<domain::common::SourceId>{}(key.source_id) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= std::hash<int>{}(static_cast<int>(key.direction)) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            return seed;
        }
    };
}
#endif //CLEANGRADUATOR_CALIBRATIONCELLKEY_H