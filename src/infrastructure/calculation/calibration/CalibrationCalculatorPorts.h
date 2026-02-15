#ifndef CLEANGRADUATOR_LINEARRESULTCALCULATORPORTS_H
#define CLEANGRADUATOR_LINEARRESULTCALCULATORPORTS_H
#include "domain/ports/logging/ILogger.h"

namespace infra::calc {
    struct CalibrationCalculatorPorts {
        domain::ports::ILogger &logger;
    };
}

#endif //CLEANGRADUATOR_LINEARRESULTCALCULATORPORTS_H