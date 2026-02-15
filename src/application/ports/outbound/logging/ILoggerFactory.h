#ifndef CLEANGRADUATOR_ILOGGERFACTORY_H
#define CLEANGRADUATOR_ILOGGERFACTORY_H
#include "domain/ports/logging/ILogger.h"

namespace application::ports {
    struct ILoggerFactory {
        virtual domain::ports::ILogger* create() = 0;
        virtual ~ILoggerFactory() = default;
    };
}

#endif //CLEANGRADUATOR_ILOGGERFACTORY_H