#ifndef CLEANGRADUATOR_USECASESBOOTSTRAP_H
#define CLEANGRADUATOR_USECASESBOOTSTRAP_H
#include <memory>
#include "domain/ports/calibration/result/ICalibrationResultSource.h"


namespace application::orchestrators {
    class MotorControlInteractor;
    class CalibrationContextProvider;
    class CalibrationOrchestrator;
}
namespace application::ports {
    struct IBatchContextProvider;
    class ICalibrationResultSaver;
}

namespace domain::ports {
    class ICalibrationStrategy;
    struct ICalibrationRecorder;
}

namespace application::usecase {
    class CalibrationSessionControl;
    class CloseAllCameras;
    class OpenAllCameras;
    class OpenSelectedCameras;
    class SaveCalibrationResult;
}

class ApplicationBootstrap;

class UseCasesBootstrap {
public:
    explicit UseCasesBootstrap(ApplicationBootstrap& application);
    ~UseCasesBootstrap();

    void initialize();

    // Камеры
    std::unique_ptr<application::usecase::OpenSelectedCameras> open_selected_cameras;
    std::unique_ptr<application::usecase::OpenAllCameras> open_all_cameras;
    std::unique_ptr<application::usecase::CloseAllCameras> close_all_cameras;
    std::unique_ptr<application::usecase::CalibrationSessionControl> calibration_session_control;
    std::unique_ptr<application::usecase::SaveCalibrationResult> save_calibration_result;

    std::unique_ptr<application::orchestrators::MotorControlInteractor> motor_control_interactor;
    std::unique_ptr<application::orchestrators::CalibrationContextProvider> calibration_context_provider;
    std::unique_ptr<application::orchestrators::CalibrationOrchestrator> calibration_process_orchestrator;
    std::unique_ptr<application::ports::IBatchContextProvider> batch_context_provider;
    std::unique_ptr<application::ports::ICalibrationResultSaver> calibration_result_saver;

    // Конфигурируемы объект
    // std::unique_ptr<application::usecase::ConfigureComponent> configure_component;

private:
    ApplicationBootstrap& app_;

    void createOpenSelectedCameras();
    void createOpenAllCameras();
    void createCloseAllCameras();

    void createMotorControlInteractor();
    void createCalibrationContextProvider();
    void createCalibrationProcessOrchestrator();
    void createCalibrationSessionControl();
    void createBatchContextProvider();
    void createCalibrationResultSaver();
    void createSaveCalibrationResult();

};


#endif //CLEANGRADUATOR_USECASESBOOTSTRAP_H
