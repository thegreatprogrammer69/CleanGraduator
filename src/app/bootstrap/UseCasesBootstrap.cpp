#include "UseCasesBootstrap.h"

#include "ApplicationBootstrap.h"
#include "application/orchestrators/calibration/process/CalibrationProcessOrchestrator.h"
#include "application/orchestrators/calibration/process/CalibrationProcessOrchestratorPorts.h"
#include "application/orchestrators/calibration/session/CalibrationSessionController.h"
#include "application/orchestrators/calibration/session/CalibrationSessionControllerPorts.h"
#include "application/orchestrators/motor/MotorControlInteractor.h"
#include "application/orchestrators/settings/CalibrationSettingsQuery.h"
#include "application/usecases/cameras/CloseAllCameras.h"
#include "application/usecases/cameras/OpenAllCameras.h"
#include "application/usecases/cameras/OpenSelectedCameras.h"
#include "domain/ports/calibration/recording/ICalibrationRecorder.h"
#include "domain/ports/calibration/strategy/ICalibrationStrategy.h"
#include "infrastructure/calibration/strats/stand4/Stand4CalibrationStrategy.h"

using namespace application::usecase;
using namespace application::orchestrators;

namespace {
class InMemoryCalibrationRecorder final : public domain::ports::ICalibrationRecorder {
public:
    void beginSession(int, domain::common::MotorDirection) override {}
    void pushPressure(float, float) override {}
    void pushAngle(domain::common::AngleSourceId, float, float) override {}
    void endSession() override {}
};
} // namespace

UseCasesBootstrap::UseCasesBootstrap(ApplicationBootstrap &application): app_(application) {}

UseCasesBootstrap::~UseCasesBootstrap() {
}

void UseCasesBootstrap::initialize() {
    createOpenSelectedCameras();
    createOpenAllCameras();
    createCloseAllCameras();

    createMotorControlInteractor();
    createCalibrationSettingsQuery();
    createCalibrationStrategy();
    createCalibrationRecorder();
    createCalibrationProcessOrchestrator();
    createCalibrationSessionController();
}

void UseCasesBootstrap::createOpenSelectedCameras() {
    open_selected_cameras = std::make_unique<OpenSelectedCameras>(*app_.video_source_manager);
}

void UseCasesBootstrap::createOpenAllCameras() {
    open_all_cameras = std::make_unique<OpenAllCameras>(*app_.video_source_manager);
}

void UseCasesBootstrap::createCloseAllCameras() {
    close_all_cameras = std::make_unique<CloseAllCameras>(*app_.video_source_manager);
}

void UseCasesBootstrap::createMotorControlInteractor() {
    motor_control_interactor = std::make_unique<MotorControlInteractor>(*app_.motor_driver);
}

void UseCasesBootstrap::createCalibrationSettingsQuery() {
    CalibrationSettingsQueryPorts ports{
        *app_.info_settings_storage,
        *app_.displacement_catalog,
        *app_.gauge_catalog,
        *app_.precision_catalog,
        *app_.pressure_unit_catalog,
        *app_.printer_catalog
    };

    calibration_settings_query = std::make_unique<CalibrationSettingsQuery>(ports);
}

void UseCasesBootstrap::createCalibrationStrategy() {
    calibration_strategy = std::make_unique<infra::calib::stand4::Stand4CalibrationStrategy>();
}

void UseCasesBootstrap::createCalibrationRecorder() {
    calibration_recorder = std::make_unique<InMemoryCalibrationRecorder>();
}

void UseCasesBootstrap::createCalibrationProcessOrchestrator() {
    CalibrationProcessOrchestratorPorts ports{
        app_.createLogger("CalibrationProcessOrchestrator"),
        *app_.pressure_source,
        *app_.videoangle_sources_storage,
        *app_.motor_driver,
        *app_.valve_driver,
        *calibration_strategy,
        *calibration_recorder,
        *app_.calibration_lifecycle
    };

    calibration_process_orchestrator = std::make_unique<CalibrationProcessOrchestrator>(ports);
}

void UseCasesBootstrap::createCalibrationSessionController() {
    CalibrationSessionControllerPorts ports{
        app_.createLogger("CalibrationSessionController"),
        *calibration_settings_query
    };

    calibration_session_controller = std::make_unique<CalibrationSessionController>(ports, *calibration_process_orchestrator);
}
