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
    mvvm::VideoSourceGridViewModel::Slots slots{
        {0, 0, *sourceViewModels.at(0)}, {0, 1, *sourceViewModels.at(std::min<size_t>(1, sourceViewModels.size() - 1))},
        {1, 0, *sourceViewModels.at(0)}, {1, 1, *sourceViewModels.at(std::min<size_t>(1, sourceViewModels.size() - 1))},
        {2, 0, *sourceViewModels.at(0)}, {2, 1, *sourceViewModels.at(std::min<size_t>(1, sourceViewModels.size() - 1))},
        {3, 0, *sourceViewModels.at(0)}, {3, 1, *sourceViewModels.at(std::min<size_t>(1, sourceViewModels.size() - 1))},
    };

    return std::make_unique<mvvm::VideoSourceGridViewModel>(std::move(slots), 4, 2, 4.0 / 3.0);
}

std::unique_ptr<application::services::VideoSourceGridService> ViewModelsModule::createGridService(
    std::vector<domain::ports::IVideoSource*> sources)
{
    using VideoSource = application::services::VideoSourceGridService::VideoSource;
    std::vector<VideoSource> videoSources;
    videoSources.reserve(sources.size());

    for (size_t index = 0; index < sources.size(); ++index) {
        videoSources.push_back(VideoSource{.index = static_cast<int>(index + 1), .video_source = *sources.at(index)});
    }

    return std::make_unique<application::services::VideoSourceGridService>(std::move(videoSources));
}

std::unique_ptr<application::usecases::ApplyCameraGridSettings> ViewModelsModule::createSettingsUseCase(
    domain::ports::ILogger& logger,
    infrastructure::settings::QtCameraGridSettingsRepository& settingsRepository,
    application::services::VideoSourceGridService& gridService,
    mvvm::VideoSourceViewModel& crosshairListener)
{
    application::usecases::ApplyCameraGridSettingsPorts ports{
        .logger = logger,
        .settings_repo = settingsRepository,
        .crosshair_listener = crosshairListener,
    };

    application::usecases::ApplyCameraGridSettingsServices services{
        .grid_service = gridService,
    };

    return std::make_unique<application::usecases::ApplyCameraGridSettings>(ports, services);
}

std::unique_ptr<mvvm::VideoSourceGridSettingsViewModel> ViewModelsModule::createSettingsViewModel(
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
