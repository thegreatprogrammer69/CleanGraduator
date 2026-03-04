#ifndef CLEANGRADUATOR_CALIBRATIONSTRATEGYPORTS_H
#define CLEANGRADUATOR_CALIBRATIONSTRATEGYPORTS_H

namespace domain::ports {
    struct ILogger;
}

namespace infra::calib {
    struct CalibrationStrategyPorts {
        domain::ports::ILogger& logger;
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONSTRATEGYPORTS_H