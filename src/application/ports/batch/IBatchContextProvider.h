#ifndef CLEANGRADUATOR_IBATCHNUMBERPROVIDER_H
#define CLEANGRADUATOR_IBATCHNUMBERPROVIDER_H
#include <optional>

#include "application/models/info/BatchContext.h"

namespace application::ports {
    struct IBatchContextProvider {
        virtual ~IBatchContextProvider() = default;
        virtual std::optional<models::BatchContext> previewNext() const = 0;
        virtual std::optional<models::BatchContext> allocateNext() = 0;
    };
}

#endif //CLEANGRADUATOR_IBATCHNUMBERPROVIDER_H
