#ifndef CLEANGRADUATOR_VIEWMODELSBOOTSTRAP_H
#define CLEANGRADUATOR_VIEWMODELSBOOTSTRAP_H
#include <memory>
#include <vector>

#include "viewmodels/MainWindowViewModel.h"
#include "viewmodels/settings/CameraGridSettingsViewModel.h"
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

    // Сетка камер
    std::vector<std::unique_ptr<mvvm::VideoSourceViewModel>> video_source;
    std::unique_ptr<mvvm::VideoSourceGridViewModel> video_source_grid;

    // Настройки
    std::unique_ptr<mvvm::CameraGridSettingsViewModel> camera_grid_settings;
    std::unique_ptr<mvvm::SettingsViewModel> settings;

    // Главное окно
    std::unique_ptr<mvvm::MainWindowViewModel> main_window;

private:
    std::vector<std::unique_ptr<mvvm::VideoSourceViewModel>> createVideoSourceViewModels();
    std::unique_ptr<mvvm::VideoSourceGridViewModel> createVideoSourceGrid();

    std::unique_ptr<mvvm::CameraGridSettingsViewModel> createCameraGridSettings();
    std::unique_ptr<mvvm::SettingsViewModel> createSettings();

    std::unique_ptr<mvvm::MainWindowViewModel> createMainWindow();
};

#endif //CLEANGRADUATOR_VIEWMODELSBOOTSTRAP_H