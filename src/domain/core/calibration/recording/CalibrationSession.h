#ifndef CLEANGRADUATOR_CALIBRATIONSESSION_H
#define CLEANGRADUATOR_CALIBRATIONSESSION_H
#include <unordered_map>

#include "CalibrationSessionId.h"
#include "TimeSeries.h"
#include "domain/core/angle/SourceId.h"

namespace domain::common {
    struct CalibrationSession {
        CalibrationSessionId id;
        PressureSeries pressure_series{};
        std::unordered_map<SourceId, AngleSeries> angle_series{};
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONSESSION_H