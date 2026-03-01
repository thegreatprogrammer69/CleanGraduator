// #include <QApplication>
// #include "ControlPanelWidget.h"
// #include "application/orchestrators/calibration/process/CalibrationOrchestrator.h"
// #include "infrastructure/logging/ConsoleLogger.h"
// #include "infrastructure/motor/g540/as_lpt/G540LptMotorDriver.h"
// #include "infrastructure/pressure/fake/FakePressureSource.h"
//
// int main(int argc, char *argv[])
// {
//     QApplication a(argc, argv);
//     infra::logging::ConsoleLogger logger;
//
//     infra::motor::MotorDriverPorts ports {
//         .logger = logger
//     };
//
//     infra::motors::G540LptMotorDriverConfig config;
//
//     infra::motor::G540LptMotorDriver motor_driver(ports, config);
//
//     /*
//     *        domain::ports::ILogger& logger;
//         domain::ports::IPressureSource& pressure_source;
//         VideoSourceManager& source_manager;
//         ports::IVideoAngleSourcesStorage& source_storage;
//         domain::ports::IMotorDriver& motor_driver;
//         domain::ports::ISessionClock& session_clock;
//         domain::ports::ICalibrationStrategy& strategy;
//         domain::ports::ICalibrationRecorder& recorder;
//      */
//
//     infra::pressure::FakePressureSource pressure_source;
//
//     application::orchestrators::CalibrationOrchestratorPorts orch_ports {
//         logger,
//
//     };
//
//     application::orchestrators::CalibrationOrchestrator orchestrator();
//
//     return a.exec();
// }
