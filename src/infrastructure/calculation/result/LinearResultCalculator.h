#ifndef CLEANGRADUATOR_LINEARRESULTCALCULATOR_H
#define CLEANGRADUATOR_LINEARRESULTCALCULATOR_H
#include "domain/ports/inbound/IResultCalculator.h"

namespace infra::calculation {
    struct LinearResultCalculator : domain::ports::IResultCalculator {
        domain::common::Result calculate(const domain::common::MeasurementSeries &, const domain::common::PressurePoints &) const override;
    };
}

#endif //CLEANGRADUATOR_LINEARRESULTCALCULATOR_H