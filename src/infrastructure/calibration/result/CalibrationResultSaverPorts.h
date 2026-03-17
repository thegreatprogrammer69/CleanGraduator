#ifndef CLEANGRADUATOR_CALIBRATIONRESULTSAVERPORTS_H
#define CLEANGRADUATOR_CALIBRATIONRESULTSAVERPORTS_H

namespace domain::ports {
    struct ILogger;
}

namespace application::ports {
    struct IBatchContextProvider;
}

namespace infra::calib {
    struct CalibrationResultSaverPorts {
        domain::ports::ILogger& logger;
        application::ports::IBatchContextProvider& batch_context_provider;
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONRESULTSAVERPORTS_H