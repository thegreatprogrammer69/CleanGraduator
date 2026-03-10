#ifndef CLEANGRADUATOR_CALIBRATIONLAYOUT_H
#define CLEANGRADUATOR_CALIBRATIONLAYOUT_H
#include <vector>

#include "domain/core/angle/SourceId.h"
#include "domain/core/calibration/recording/PointId.h"
#include "domain/core/drivers/motor/MotorDirection.h"

namespace domain::common {
    struct CalibrationLayout {
        std::vector<MotorDirection> directions{};
        std::vector<PointId> points{};
        std::vector<SourceId> sources{};

        size_t getWidth() const { return points.size() * sources.size(); }
        size_t getHeight() const { return directions.size(); }
        size_t getTotalCells() const { return getWidth() * getHeight(); }
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONLAYOUT_H