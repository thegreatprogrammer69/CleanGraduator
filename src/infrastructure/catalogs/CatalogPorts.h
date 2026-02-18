#ifndef CLEANGRADUATOR_CATALOGPORTS_H
#define CLEANGRADUATOR_CATALOGPORTS_H
#include "domain/ports/logging/ILogger.h"

namespace infra::catalogs {
    struct CatalogPorts {
        domain::ports::ILogger& logger;
    };
}

#endif //CLEANGRADUATOR_CATALOGPORTS_H