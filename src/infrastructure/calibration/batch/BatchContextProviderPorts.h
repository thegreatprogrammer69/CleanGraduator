#ifndef CLEANGRADUATOR_BATCHCONTEXTPROVIDERPORTS_H
#define CLEANGRADUATOR_BATCHCONTEXTPROVIDERPORTS_H

namespace domain::ports {
    struct ILogger;
}

namespace application::orchestrators {
    class CalibrationSettingsQuery;
}

namespace infra::calib {
    struct BatchContextProviderPorts {
        domain::ports::ILogger& logger_;
        application::orchestrators::CalibrationSettingsQuery& settings_query_;
    };
}

#endif //CLEANGRADUATOR_BATCHCONTEXTPROVIDERPORTS_H