#ifndef CLEANGRADUATOR_RETURN_TO_HOME_H
#define CLEANGRADUATOR_RETURN_TO_HOME_H

namespace domain::ports {
    struct IValveDriver;
}

namespace domain::ports {
    struct IMotorDriver;
    struct ILogger;
}

namespace infra::calib::detail {
    bool executeHoming(
        domain::ports::ILogger& logger,
        domain::ports::IMotorDriver& motor_driver,
        domain::ports::IValveDriver& valve_driver
    );
}

#endif //CLEANGRADUATOR_RETURN_TO_HOME_H