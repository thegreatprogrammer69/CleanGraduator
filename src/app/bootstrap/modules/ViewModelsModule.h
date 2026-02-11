#ifndef CLEANGRADUATOR_VIEWMODELSMODULE_H
#define CLEANGRADUATOR_VIEWMODELSMODULE_H

#include <memory>
#include <vector>

#include "application/services/CameraGridService.h"
#include "application/usecases/settings/ApplyGridString.h"

namespace domain::ports {
struct IVideoSource;
}

namespace infrastructure::settings {
class IGridSettingsRepository;
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

    static std::unique_ptr<application::services::CameraGridService> createGridService(
        application::ports::IVideoSourceGridSettingsRepository& settings_repo,
        std::vector<domain::ports::IVideoSource*> video_sources,
        std::vector<application::ports::IVideoSourceCrosshairListener*> crosshair_listeners,
        std::vector<application::ports::IVideoSourceLifecycleObserver*> lifecycle_observers);

    static std::unique_ptr<application::usecases::ApplyCameraGridSettings> createCameraGridSettingsUseCase(
        domain::ports::ILogger& logger,
        application::services::CameraGridService& gridService);

    static std::unique_ptr<mvvm::VideoSourceGridSettingsViewModel> createCameraGridSettingsViewModel(
        application::usecases::ApplyCameraGridSettings& useCase);

    static std::unique_ptr<mvvm::MainWindowViewModel> createMainWindowViewModel(
        mvvm::VideoSourceGridViewModel& grid,
        mvvm::VideoSourceGridSettingsViewModel& settings);
};

#endif //CLEANGRADUATOR_VIEWMODELSMODULE_H
