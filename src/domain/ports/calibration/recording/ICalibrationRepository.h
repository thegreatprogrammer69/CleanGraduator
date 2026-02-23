#ifndef CLEANGRADUATOR_ICALIBRATIONREPOSITORY1_H
#define CLEANGRADUATOR_ICALIBRATIONREPOSITORY1_H
#include <optional>

#include "domain/core/calibration/recording/TimeSeries.h"
#include "domain/core/drivers/motor/MotorDirection.h"

namespace domain::ports {
    struct ICalibrationRepository {
        virtual ~ICalibrationRepository() = default;
        virtual std::optional<common::AngleSeries> angleSeries(int point, common::MotorDirection direction, int idx) = 0;
        virtual std::optional<common::PressureSeries> pressureSeries(int point, common::MotorDirection direction) = 0;
    };
}

#endif //CLEANGRADUATOR_ICALIBRATIONREPOSITORY1_H