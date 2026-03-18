#ifndef CLEANGRADUATOR_CALIBRATIONRESULTSAVERPORTS_H
#define CLEANGRADUATOR_CALIBRATIONRESULTSAVERPORTS_H

namespace domain::ports {
    struct ILogger;
}

namespace infra::calib {
    struct CalibrationResultSaverPorts {
        domain::ports::ILogger& logger;
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONRESULTSAVERPORTS_H