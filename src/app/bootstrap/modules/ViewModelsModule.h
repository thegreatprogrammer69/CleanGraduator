#ifndef CLEANGRADUATOR_VIEWMODELSMODULE_H
#define CLEANGRADUATOR_VIEWMODELSMODULE_H

#include <memory>
#include <vector>

#include "application/services/VideoSourceGridService.h"
#include "application/usecases/settings/ApplyCameraGridSettings.h"

namespace domain::ports {
struct IVideoSource;
}

namespace infrastructure::settings {
class QtCameraGridSettingsRepository;
}

namespace mvvm {
class VideoSourceViewModel;
class VideoSourceGridViewModel;
class VideoSourceGridSettingsViewModel;
class MainWindowViewModel;
}

struct ViewModelsModule {
    static std::vector<std::unique_ptr<mvvm::VideoSourceViewModel>> createVideoSourceViewModels(
        std::vector<domain::ports::IVideoSource*> sources);

    static std::unique_ptr<mvvm::VideoSourceGridViewModel> createGridViewModel(
        const std::vector<std::unique_ptr<mvvm::VideoSourceViewModel>>& sourceViewModels);

    static std::unique_ptr<application::services::VideoSourceGridService> createGridService(
        std::vector<domain::ports::IVideoSource*> sources);

    static std::unique_ptr<application::usecases::ApplyCameraGridSettings> createSettingsUseCase(
        domain::ports::ILogger& logger,
        infrastructure::settings::QtCameraGridSettingsRepository& settingsRepository,
        application::services::VideoSourceGridService& gridService,
        mvvm::VideoSourceViewModel& crosshairListener);

    static std::unique_ptr<mvvm::VideoSourceGridSettingsViewModel> createSettingsViewModel(
        application::usecases::ApplyCameraGridSettings& useCase);

    static std::unique_ptr<mvvm::MainWindowViewModel> createMainWindowViewModel(
        mvvm::VideoSourceGridViewModel& grid,
        mvvm::VideoSourceGridSettingsViewModel& settings);
};

#endif //CLEANGRADUATOR_VIEWMODELSMODULE_H
