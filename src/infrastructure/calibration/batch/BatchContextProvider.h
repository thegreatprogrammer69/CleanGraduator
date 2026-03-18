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
        std::optional<application::models::BatchContext> previewNext() const override;
        std::optional<application::models::BatchContext> allocateNext() override;

    private:
        std::optional<application::models::BatchContext> resolveNextBatch(bool create_directory) const;

        fmt::Logger logger_;
        application::orchestrators::CalibrationContextProvider& context_provider_;
    };
}

#endif //CLEANGRADUATOR_BATCHCONTEXTPROVIDER_H
