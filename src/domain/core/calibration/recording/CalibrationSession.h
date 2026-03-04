#ifndef CLEANGRADUATOR_CALIBRATIONSESSION_H
#define CLEANGRADUATOR_CALIBRATIONSESSION_H
#include <unordered_map>

#include "CalibrationSessionId.h"
#include "TimeSeries.h"
#include "domain/core/angle/AngleSourceId.h"

namespace domain::common {
    struct CalibrationSession {
        float pressure_point;
        MotorDirection direction;
        PressureSeries pressure_series{};
        std::unordered_map<AngleSourceId, AngleSeries> angle_series{};
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONSESSION_H