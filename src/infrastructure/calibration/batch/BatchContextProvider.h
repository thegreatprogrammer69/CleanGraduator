#ifndef CLEANGRADUATOR_BATCHCONTEXTPROVIDER_H
#define CLEANGRADUATOR_BATCHCONTEXTPROVIDER_H
#include "BatchContextProviderPorts.h"
#include "application/ports/batch/IBatchContextProvider.h"

namespace infra::calib {
    class BatchContextProvider final : public application::ports::IBatchContextProvider {
    public:
        ~BatchContextProvider() override;
        explicit BatchContextProvider(BatchContextProviderPorts ports);

        application::models::BatchContext current() override;
    };
}


#endif //CLEANGRADUATOR_BATCHCONTEXTPROVIDER_H