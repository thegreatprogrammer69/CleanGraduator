#ifndef CLEANGRADUATOR_P540PORTS_H
#define CLEANGRADUATOR_P540PORTS_H
#include "../../../domain/ports/outbound/ILogger.h"

namespace infra::hardware {
    struct G540Ports {
        ILogger& logger;
    };
}

#endif //CLEANGRADUATOR_P540PORTS_H