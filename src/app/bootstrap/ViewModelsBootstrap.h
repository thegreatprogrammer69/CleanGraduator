#ifndef CLEANGRADUATOR_VIEWMODELSBOOTSTRAP_H
#define CLEANGRADUATOR_VIEWMODELSBOOTSTRAP_H
#include <memory>
#include <vector>

namespace mvvm {
    class ControlViewModel;
    class MotorControlViewModel;
    class DualValveControlViewModel;
    class AppStatusBarViewModel;
    class MotorDriverStatusViewModel;
    class PressureSensorStatusBarViewModel;
    class MainWindowViewModel;
    class LogViewerViewModel;
    class SettingsViewModel;
    class InfoSettingsViewModel;
    class CameraGridSettingsViewModel;
    class VideoSourceGridViewModel;
    class VideoSourceViewModel;
}

class UseCasesBootstrap;
class ApplicationBootstrap;

class ViewModelsBootstrap {
public:
    ViewModelsBootstrap(ApplicationBootstrap& application, UseCasesBootstrap& use_cases);
    ~ViewModelsBootstrap();

    void initialize();

    std::vector<std::unique_ptr<mvvm::VideoSourceViewModel>> video_source_view_models;
    std::unique_ptr<mvvm::VideoSourceGridViewModel> video_source_grid;

    std::unique_ptr<mvvm::CameraGridSettingsViewModel> camera_grid_settings;
    std::unique_ptr<mvvm::InfoSettingsViewModel> info_settings;
    std::unique_ptr<mvvm::SettingsViewModel> settings;

    std::unique_ptr<mvvm::AppStatusBarViewModel> app_status_bar;
    std::unique_ptr<mvvm::MotorDriverStatusViewModel> motor_driver_status;
    std::unique_ptr<mvvm::PressureSensorStatusBarViewModel> pressure_sensor_status;


    std::unique_ptr<mvvm::DualValveControlViewModel> dual_valve_control;
    std::unique_ptr<mvvm::MotorControlViewModel> motor_control;
    std::unique_ptr<mvvm::ControlViewModel> control;

    std::unique_ptr<mvvm::LogViewerViewModel> log_viewer;

    std::unique_ptr<mvvm::MainWindowViewModel> main_window;

private:
    ApplicationBootstrap& app_;
    UseCasesBootstrap& use_cases_;

    void createVideoSourceViewModels();
    void createVideoSourceGrid();

    void createCameraGridSettings();
    void createInfoSettings();
    void createSettings();

    void createStatusBar();
    void createMotorDriverStatus();
    void createPressureSensorStatus();

    void createDualValveControl();
    void createMotorControl();
    void createControl();

    void createLogViewer();

    void createMainWindow();
};

#endif //CLEANGRADUATOR_VIEWMODELSBOOTSTRAP_H
