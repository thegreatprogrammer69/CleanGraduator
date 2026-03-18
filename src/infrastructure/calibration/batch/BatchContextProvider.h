#ifndef CLEANGRADUATOR_BATCHCONTEXTPROVIDER_H
#define CLEANGRADUATOR_BATCHCONTEXTPROVIDER_H
#include <optional>

#include "BatchContextProviderPorts.h"
#include "application/ports/batch/IBatchContextProvider.h"
#include "domain/fmt/Logger.h"

namespace infra::calib {
    class BatchContextProvider final
        : public application::ports::IBatchContextProvider
        , public application::ports::IBatchContextAllocator {
    public:
        ~BatchContextProvider() override;
        explicit BatchContextProvider(BatchContextProviderPorts ports);

        std::optional<application::models::BatchContext> current() override;
        std::optional<application::models::BatchContext> allocate() override;

    private:
        std::optional<application::models::BatchContext> createContext();

    private:
        fmt::Logger logger_;
        application::orchestrators::CalibrationSettingsQuery& settings_query_;
        std::optional<application::models::BatchContext> current_context_;
    };
}


#endif //CLEANGRADUATOR_BATCHCONTEXTPROVIDER_H
