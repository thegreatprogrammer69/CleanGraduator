#ifndef CLEANGRADUATOR_VIEWMODELSBOOTSTRAP_H
#define CLEANGRADUATOR_VIEWMODELSBOOTSTRAP_H
#include <memory>
#include <vector>

#include "viewmodels/MainWindowViewModel.h"
#include "viewmodels/settings/CameraGridSettingsViewModel.h"
#include "viewmodels/settings/InfoSettingsViewModel.h"
#include "viewmodels/settings/SettingsViewModel.h"
#include "viewmodels/video/VideoSourceGridViewModel.h"
#include "viewmodels/video/VideoSourceViewModel.h"

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

    void createLogViewer();

    void createMainWindow();
};

#endif //CLEANGRADUATOR_VIEWMODELSBOOTSTRAP_H
