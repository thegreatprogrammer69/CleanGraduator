#ifndef CLEANGRADUATOR_LINEARRESULTCALCULATORPORTS_H
#define CLEANGRADUATOR_LINEARRESULTCALCULATORPORTS_H
#include "domain/ports/outbound/ILogger.h"

namespace infra::calculation {
    struct LinearResultCalculatorPorts {
        domain::ports::ILogger &logger;
    };
}

#endif //CLEANGRADUATOR_LINEARRESULTCALCULATORPORTS_H