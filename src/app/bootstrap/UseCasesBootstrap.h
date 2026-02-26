#ifndef CLEANGRADUATOR_USECASESBOOTSTRAP_H
#define CLEANGRADUATOR_USECASESBOOTSTRAP_H
#include <memory>


namespace application::orchestrators {
    class MotorControlInteractor;
    class CalibrationSettingsQuery;
    class CalibrationOrchestrator;
    class CalibrationSessionController;
}

namespace domain::ports {
    class ICalibrationStrategy;
    struct ICalibrationRecorder;
}

namespace application::usecase {
    class CloseAllCameras;
    class OpenAllCameras;
    class OpenSelectedCameras;
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

    std::unique_ptr<application::orchestrators::MotorControlInteractor> motor_control_interactor;
    std::unique_ptr<application::orchestrators::CalibrationSettingsQuery> calibration_settings_query;
    std::unique_ptr<domain::ports::ICalibrationStrategy> calibration_strategy;
    std::unique_ptr<domain::ports::ICalibrationRecorder> calibration_recorder;
    std::unique_ptr<application::orchestrators::CalibrationOrchestrator> calibration_process_orchestrator;
    std::unique_ptr<application::orchestrators::CalibrationSessionController> calibration_session_controller;


    // Конфигурируемы объект
    // std::unique_ptr<application::usecase::ConfigureComponent> configure_component;

private:
    ApplicationBootstrap& app_;

    void createOpenSelectedCameras();
    void createOpenAllCameras();
    void createCloseAllCameras();

    void createMotorControlInteractor();
    void createCalibrationSettingsQuery();
    void createCalibrationStrategy();
    void createCalibrationRecorder();
    void createCalibrationProcessOrchestrator();
    void createCalibrationSessionController();

};


#endif //CLEANGRADUATOR_USECASESBOOTSTRAP_H
