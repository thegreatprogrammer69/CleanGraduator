#ifndef CLEANGRADUATOR_BATCHCONTEXTPROVIDER_H
#define CLEANGRADUATOR_BATCHCONTEXTPROVIDER_H
#include "BatchContextProviderPorts.h"
#include "application/ports/batch/IBatchContextProvider.h"
#include "domain/fmt/Logger.h"

namespace infra::calib {
    class BatchContextProvider final : public application::ports::IBatchContextProvider {
    public:
        ~BatchContextProvider() override;
        explicit BatchContextProvider(BatchContextProviderPorts ports);
        std::optional<application::models::BatchContext> current() override;

    private:
        fmt::Logger logger_;
        application::orchestrators::CalibrationSettingsQuery& settings_query_;
    };
}


#endif //CLEANGRADUATOR_BATCHCONTEXTPROVIDER_H