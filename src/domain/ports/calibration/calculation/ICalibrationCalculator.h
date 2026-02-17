#ifndef CLEANGRADUATOR_ICALIBRATIONCALCULATOR_H
#define CLEANGRADUATOR_ICALIBRATIONCALCULATOR_H
#include "CalibrationCalculatorInput.h"
#include "domain/core/calibration/result/CalibrationResult.h"

namespace domain::common {
    class TimeSeries;
}

namespace domain::ports {
    class ICalibrationCalculator {
    public:
        virtual ~ICalibrationCalculator() = default;
        virtual common::CalibrationResult calculate(const common::CalibrationCalculatorInput&) const = 0;
    };
}

#endif //CLEANGRADUATOR_ICALIBRATIONCALCULATOR_H