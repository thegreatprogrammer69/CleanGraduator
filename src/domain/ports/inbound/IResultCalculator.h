#ifndef CLEANGRADUATOR_IRESULTCALCULATOR_H
#define CLEANGRADUATOR_IRESULTCALCULATOR_H
#include "domain/core/result/Result.h"
#include "domain/core/measurement/PressurePoints.h"

namespace domain::common {
    class MeasurementSeries;
}

namespace domain::ports {
    struct IResultCalculator {
        virtual ~IResultCalculator() = default;
        virtual common::Result calculate(const common::MeasurementSeries &, const common::PressurePoints &) const = 0;
    };
}

#endif //CLEANGRADUATOR_IRESULTCALCULATOR_H