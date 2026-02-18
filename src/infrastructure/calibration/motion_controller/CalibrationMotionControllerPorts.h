#ifndef CLEANGRADUATOR_CALIBRATIONMOTIONCONTROLLER1PORTS_H
#define CLEANGRADUATOR_CALIBRATIONMOTIONCONTROLLER1PORTS_H


namespace domain::ports {
    struct IMotorDriver;
    struct ILogger;
    struct IValveDriver;
}

namespace infra::calib {
    struct CalibrationMotionControllerPorts {
        domain::ports::ILogger& logger;
        domain::ports::IMotorDriver& motor_driver;
        domain::ports::IValveDriver& dual_valve_driver;
    };
}


#endif //CLEANGRADUATOR_CALIBRATIONMOTIONCONTROLLER1PORTS_H