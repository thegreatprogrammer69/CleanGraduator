#include "UseCasesBootstrap.h"

#include "ApplicationBootstrap.h"
#include "application/orchestrators/calibration/process/CalibrationProcessOrchestrator.h"
#include "application/orchestrators/calibration/process/CalibrationProcessOrchestratorPorts.h"
#include "application/orchestrators/calibration/session/CalibrationSessionController.h"
#include "application/orchestrators/calibration/session/CalibrationSessionControllerPorts.h"
#include "application/orchestrators/motor/MotorControlInteractor.h"
#include "application/orchestrators/settings/CalibrationSettingsQuery.h"
#include "application/usecases/cameras/OpenSelectedCameras.h"
#include "application/usecases/cameras/OpenAllCameras.h"
#include "application/usecases/cameras/CloseAllCameras.h"
#include "domain/core/calibration/strategy/CalibrationFeedContext.h"
#include "domain/ports/calibration/recording/ICalibrationRecorder.h"
#include "domain/ports/calibration/strategy/ICalibrationStrategy.h"


using namespace application::usecase;
using namespace application::orchestrators;

namespace {

class NoopCalibrationRecorder final : public domain::ports::ICalibrationRecorder {
public:
    void beginSession(int, domain::common::MotorDirection) override {}
    void pushPressure(float, float) override {}
    void pushAngle(domain::common::AngleSourceId, float, float) override {}
    void endSession() override {}
};

class NoopCalibrationStrategy final : public domain::ports::ICalibrationStrategy {
public:
    void bind(
        domain::ports::IMotorDriver&,
        domain::ports::IValveDriver&,
        domain::ports::ICalibrationRecorder&) override {}

    void begin(const domain::common::CalibrationBeginContext&) override {
        running_ = true;
    }

    domain::ports::CalibrationDecisionType feed(const domain::common::CalibrationFeedContext&) override {
        return domain::ports::CalibrationDecisionType::None;
    }

    void end() override {
        running_ = false;
    }

    bool isRunning() const override {
        return running_;
    }

private:
    bool running_{false};
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

void UseCasesBootstrap::createCalibrationSessionController() {
    CalibrationSettingsQueryPorts settings_ports {
        *app_.info_settings_storage,
        *app_.displacement_catalog,
        *app_.gauge_catalog,
        *app_.precision_catalog,
        *app_.pressure_unit_catalog,
        *app_.printer_catalog,
    };
    calibration_settings_query = std::make_unique<CalibrationSettingsQuery>(settings_ports);

    static NoopCalibrationRecorder recorder;
    static NoopCalibrationStrategy strategy;

    CalibrationProcessOrchestratorPorts process_ports {
        app_.createLogger("CalibrationProcessOrchestrator"),
        *app_.pressure_source,
        *app_.videoangle_sources_storage,
        *app_.motor_driver,
        *app_.valve_driver,
        strategy,
        recorder,
        *app_.calibration_lifecycle,
    };

    calibration_process_orchestrator = std::make_unique<CalibrationProcessOrchestrator>(process_ports);

    CalibrationSessionControllerPorts controller_ports {
        app_.createLogger("CalibrationSessionController"),
        *calibration_settings_query,
        *app_.calibration_lifecycle,
    };

    calibration_session_controller = std::make_unique<CalibrationSessionController>(
        controller_ports,
        *calibration_process_orchestrator
    );
}
