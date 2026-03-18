#ifndef CLEANGRADUATOR_USECASESBOOTSTRAP_H
#define CLEANGRADUATOR_USECASESBOOTSTRAP_H
#include <memory>
#include "domain/ports/calibration/result/ICalibrationResultSource.h"


namespace application::orchestrators {
    class MotorControlInteractor;
    class CalibrationSettingsQuery;
    class CalibrationOrchestrator;
}

namespace application::ports {
    struct IBatchContextProvider;
}

namespace domain::ports {
    class ICalibrationStrategy;
    struct ICalibrationRecorder;
}

namespace application::usecase {
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

    std::unique_ptr<application::orchestrators::MotorControlInteractor> motor_control_interactor;
    std::unique_ptr<application::orchestrators::CalibrationSettingsQuery> calibration_settings_query;
    std::unique_ptr<application::orchestrators::CalibrationOrchestrator> calibration_process_orchestrator;
    std::unique_ptr<application::ports::IBatchContextProvider> batch_context_provider;
    std::unique_ptr<application::usecase::SaveCalibrationResult> save_calibration_result;

    // Конфигурируемы объект
    // std::unique_ptr<application::usecase::ConfigureComponent> configure_component;

private:
    ApplicationBootstrap& app_;

    void createOpenSelectedCameras();
    void createOpenAllCameras();
    void createCloseAllCameras();

    void createMotorControlInteractor();
    void createCalibrationSettingsQuery();
    void createCalibrationProcessOrchestrator();
    void createBatchContextProvider();
    void createSaveCalibrationResult();

};


#endif //CLEANGRADUATOR_USECASESBOOTSTRAP_H
