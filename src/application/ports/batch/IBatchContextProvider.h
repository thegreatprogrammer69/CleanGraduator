#ifndef CLEANGRADUATOR_IBATCHNUMBERPROVIDER_H
#define CLEANGRADUATOR_IBATCHNUMBERPROVIDER_H
#include <filesystem>
#include <optional>

#include "application/models/info/BatchContext.h"

namespace application::ports {
    struct IBatchContextProvider {
        virtual ~IBatchContextProvider() = default;
        virtual std::optional<models::BatchContext> current() = 0;
    };
}

#endif //CLEANGRADUATOR_IBATCHNUMBERPROVIDER_H