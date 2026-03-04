#ifndef CLEANGRADUATOR_CALIBRATIONPROCESSORCHESTRATORPORTS_H
#define CLEANGRADUATOR_CALIBRATIONPROCESSORCHESTRATORPORTS_H
#include "domain/ports/clock/ISessionClock.h"


namespace application::ports {
    struct IVideoAngleSourcesStorage;
}

namespace domain::ports {
    struct ICalibrationRecorder;
    class ICalibrationStrategy;
    struct IMotorDriver;
    struct ILogger;
    struct IAngleSource;
    struct IPressureSource;
}

namespace application::orchestrators {
    class VideoSourceManager;

    struct CalibrationOrchestratorPorts {
        domain::ports::ILogger& logger;
        domain::ports::IPressureSource& pressure_source;
        VideoSourceManager& source_manager;
        ports::IVideoAngleSourcesStorage& source_storage;
        domain::ports::IMotorDriver& motor_driver;
        domain::ports::ISessionClock& session_clock;
        domain::ports::ICalibrationStrategy& strategy;
        domain::ports::ICalibrationRecorder& recorder;
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONPROCESSORCHESTRATORPORTS_H