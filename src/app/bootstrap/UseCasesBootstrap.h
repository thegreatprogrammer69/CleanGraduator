#ifndef CLEANGRADUATOR_USECASESBOOTSTRAP_H
#define CLEANGRADUATOR_USECASESBOOTSTRAP_H
#include <memory>


namespace application::orchestrators {
    class MotorControlInteractor;
    class CalibrationProcessOrchestrator;
    class CalibrationSessionController;
    class CalibrationSettingsQuery;
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
    std::unique_ptr<application::orchestrators::CalibrationProcessOrchestrator> calibration_process_orchestrator;
    std::unique_ptr<application::orchestrators::CalibrationSessionController> calibration_session_controller;


    // Конфигурируемы объект
    // std::unique_ptr<application::usecase::ConfigureComponent> configure_component;

private:
    ApplicationBootstrap& app_;

    void createOpenSelectedCameras();
    void createOpenAllCameras();
    void createCloseAllCameras();

    void createMotorControlInteractor();
    void createCalibrationSessionController();

};


#endif //CLEANGRADUATOR_USECASESBOOTSTRAP_H