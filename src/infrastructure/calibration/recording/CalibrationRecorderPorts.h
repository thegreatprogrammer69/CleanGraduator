#ifndef CLEANGRADUATOR_CALIBRATIONRECORDERPORTS_H
#define CLEANGRADUATOR_CALIBRATIONRECORDERPORTS_H

namespace domain::ports {
    struct ILogger;
}

namespace infra::calib {
    struct CalibrationRecorderPorts {
        domain::ports::ILogger& logger;
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONRECORDERPORTS_H