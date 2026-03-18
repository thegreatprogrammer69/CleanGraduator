#ifndef CLEANGRADUATOR_BATCHCONTEXTPROVIDERPORTS_H
#define CLEANGRADUATOR_BATCHCONTEXTPROVIDERPORTS_H

namespace domain::ports {
    struct ILogger;
}

namespace application::orchestrators {
    class CalibrationContextProvider;
}

namespace infra::calib {
    struct BatchContextProviderPorts {
        domain::ports::ILogger& logger_;
        application::orchestrators::CalibrationContextProvider& context_provider_;
    };
}

#endif //CLEANGRADUATOR_BATCHCONTEXTPROVIDERPORTS_H