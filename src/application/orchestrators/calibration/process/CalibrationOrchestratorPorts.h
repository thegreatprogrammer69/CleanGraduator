#ifndef CLEANGRADUATOR_CALIBRATIONPROCESSORCHESTRATORPORTS_H
#define CLEANGRADUATOR_CALIBRATIONPROCESSORCHESTRATORPORTS_H


namespace application::ports {
    struct IVideoAngleSourcesStorage;
}

namespace domain::ports {
    struct ICalibrationRecorder;
    class ICalibrationStrategy;
    struct IValveDriver;
    struct IMotorDriver;
    struct ILogger;
    struct IAngleSource;
    struct IPressureSource;
    struct ICalibrationLifecycle;
}

namespace application::orchestrators {
    struct CalibrationOrchestratorPorts {
        domain::ports::ILogger& logger;
        domain::ports::IPressureSource& pressure_source;
        ports::IVideoAngleSourcesStorage& angle_sources_storage;
        domain::ports::IMotorDriver& motor_driver;
        domain::ports::IValveDriver& valve_driver;
        domain::ports::ICalibrationStrategy& strategy;
        domain::ports::ICalibrationRecorder& recorder;
        domain::ports::ICalibrationLifecycle& lifecycle;
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONPROCESSORCHESTRATORPORTS_H