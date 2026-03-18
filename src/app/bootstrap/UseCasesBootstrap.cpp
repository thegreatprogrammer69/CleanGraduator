#include "UseCasesBootstrap.h"

#include "ApplicationBootstrap.h"
#include "application/orchestrators/calibration/process/CalibrationOrchestrator.h"
#include "application/orchestrators/calibration/process/CalibrationOrchestratorPorts.h"
#include "application/orchestrators/motor/MotorControlInteractor.h"
#include "application/orchestrators/settings/CalibrationContextProvider.h"
#include "application/usecases/cameras/CloseAllCameras.h"
#include "application/usecases/cameras/OpenAllCameras.h"
#include "application/usecases/cameras/OpenSelectedCameras.h"
#include "application/usecases/calibration/CalibrationSessionControl.h"
#include "application/usecases/calibration/SaveCalibrationResult.h"
#include "domain/ports/calibration/recording/ICalibrationRecorder.h"
#include "domain/ports/calibration/strategy/ICalibrationStrategy.h"
#include "infrastructure/calibration/batch/BatchContextProvider.h"
#include "infrastructure/calibration/batch/BatchContextProviderPorts.h"
#include "infrastructure/calibration/recording/in_memory/InMemoryCalibrationRecorder.h"
#include "infrastructure/calibration/result/in_file/FileCalibrationResultSaver.h"
#include "infrastructure/calibration/result/CalibrationResultSaverPorts.h"
#include "infrastructure/calibration/strats/stand4/Stand4CalibrationStrategy.h"

using namespace application::usecase;
using namespace application::orchestrators;
using namespace infra::calib;


UseCasesBootstrap::UseCasesBootstrap(ApplicationBootstrap &application): app_(application) {}

UseCasesBootstrap::~UseCasesBootstrap() {
}

void UseCasesBootstrap::initialize() {
    createOpenSelectedCameras();
    createOpenAllCameras();
    createCloseAllCameras();

    createMotorControlInteractor();
    createCalibrationContextProvider();
    createBatchContextProvider();
    createCalibrationProcessOrchestrator();
    createCalibrationSessionControl();
    createCalibrationResultSaver();
    createSaveCalibrationResult();
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

void UseCasesBootstrap::createCalibrationContextProvider() {
    CalibrationContextProviderPorts ports{
        *app_.info_settings_storage,
        *app_.displacement_catalog,
        *app_.gauge_catalog,
        *app_.precision_catalog,
        *app_.pressure_unit_catalog,
        *app_.printer_catalog
    };

    calibration_context_provider = std::make_unique<CalibrationContextProvider>(ports);
}


void UseCasesBootstrap::createCalibrationProcessOrchestrator() {
    CalibrationOrchestratorPorts ports{
        app_.createLogger("CalibrationOrchestrator"),
        *app_.pressure_source,
        *app_.video_source_manager,
        *app_.videoangle_sources_storage,
        *app_.motor_driver,
        *app_.session_clock,
        *app_.calibration_strategy,
        *app_.calibration_recorder
    };

    calibration_process_orchestrator = std::make_unique<CalibrationOrchestrator>(ports);
}

void UseCasesBootstrap::createCalibrationSessionControl() {
    calibration_session_control = std::make_unique<CalibrationSessionControl>(
        *calibration_process_orchestrator,
        *calibration_context_provider);
}

void UseCasesBootstrap::createBatchContextProvider() {
    BatchContextProviderPorts ports{
        app_.createLogger("BatchContextProvider"),
        *calibration_context_provider
    };
    batch_context_provider = std::make_unique<BatchContextProvider>(ports);
}

void UseCasesBootstrap::createCalibrationResultSaver() {
    CalibrationResultSaverPorts ports{
        app_.createLogger("FileCalibrationResultSaver"),
        *batch_context_provider
    };
    calibration_result_saver = std::make_unique<FileCalibrationResultSaver>(ports);
}

void UseCasesBootstrap::createSaveCalibrationResult() {
    application::usecase::SaveCalibrationResultDeps deps{
        *app_.calibration_result_source,
        *calibration_result_saver,
        *batch_context_provider
    };
    save_calibration_result = std::make_unique<application::usecase::SaveCalibrationResult>(deps);
}
