#include "ViewModelsModule.h"

#include <algorithm>
#include <cstddef>

#include <stdexcept>

#include "infrastructure/settings/QtCameraGridSettingsRepository.h"
#include "viewmodels/MainWindowViewModel.h"
#include "viewmodels/settings/VideoSourceGridSettingsViewModel.h"
#include "viewmodels/video/VideoSourceGridViewModel.h"
#include "viewmodels/video/VideoSourceViewModel.h"

std::vector<std::unique_ptr<mvvm::VideoSourceViewModel>> ViewModelsModule::createVideoSourceViewModels(
    std::vector<domain::ports::IVideoSource*> sources)
{
    std::vector<std::unique_ptr<mvvm::VideoSourceViewModel>> result;
    result.reserve(sources.size());
    for (auto* source : sources) {
        result.push_back(std::make_unique<mvvm::VideoSourceViewModel>(*source));
    }
    return result;
}

std::unique_ptr<mvvm::VideoSourceGridViewModel> ViewModelsModule::createGridViewModel(
    const std::vector<std::unique_ptr<mvvm::VideoSourceViewModel>>& sourceViewModels)
{
    if (sourceViewModels.empty()) {
        throw std::runtime_error("At least one camera is required to build grid view model");
    }

    using Slot = mvvm::VideoSourceGridViewModel::Slot;
    mvvm::VideoSourceGridViewModel::Slots slots{};

    for (int col = 0; col < 2; ++col) {
        for (int row = 0; row < 4; ++row) {
            int i = col * 4 + row;
            if (i >= sourceViewModels.size()) break;
            slots.push_back(Slot(row, col, *sourceViewModels[i]));
        }
    }

    return std::make_unique<mvvm::VideoSourceGridViewModel>(std::move(slots), 4, 2, 4.0 / 3.0);
}

std::unique_ptr<application::services::VideoSourceGridService> ViewModelsModule::createGridService(
    application::ports::IVideoSourceGridSettingsRepository& settings_repo,
    std::vector<domain::ports::IVideoSource*> video_sources,
    std::vector<application::ports::IVideoSourceCrosshairListener*> crosshair_listeners,
    std::vector<application::ports::IVideoSourceLifecycleObserver*> lifecycle_observers)
{
    using VideoSource = application::services::VideoSourceGridService::VideoSource;
    std::vector<VideoSource> videoSources;
    videoSources.reserve(video_sources.size());

    for (size_t index = 0; index < video_sources.size(); ++index) {
        videoSources.push_back(VideoSource{
            .index = static_cast<int>(index + 1),
            .video_source = *video_sources.at(index),
            .crosshair_listener = *crosshair_listeners.at(index),
            .lifecycle_observer = *lifecycle_observers.at(index),
        });
    }

    application::services::VideoSourceGridServicePorts ports{
        .settings_repo = settings_repo
    };

    return std::make_unique<application::services::VideoSourceGridService>(ports, std::move(videoSources));
}

std::unique_ptr<application::usecases::ApplyCameraGridSettings> ViewModelsModule::createCameraGridSettingsUseCase(
    domain::ports::ILogger& logger,
    application::services::VideoSourceGridService& gridService)
{
    application::usecases::ApplyCameraGridSettingsPorts ports{
        .logger = logger
    };

    application::usecases::ApplyCameraGridSettingsServices services{
        .grid_service = gridService,
    };

    return std::make_unique<application::usecases::ApplyCameraGridSettings>(ports, services);
}

std::unique_ptr<mvvm::VideoSourceGridSettingsViewModel> ViewModelsModule::createCameraGridSettingsViewModel(
    application::usecases::ApplyCameraGridSettings& useCase)
{
    return std::make_unique<mvvm::VideoSourceGridSettingsViewModel>(useCase);
}

std::unique_ptr<mvvm::MainWindowViewModel> ViewModelsModule::createMainWindowViewModel(
    mvvm::VideoSourceGridViewModel& grid,
    mvvm::VideoSourceGridSettingsViewModel& settings)
{
    return std::make_unique<mvvm::MainWindowViewModel>(grid, settings);
}
