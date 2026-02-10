#include "AppBootstrap.h"

#include <stdexcept>
#include <vector>

#include "application/interactors/AngleFromVideoInteractor.h"
#include "app/bootstrap/modules/DomainModule.h"
#include "app/bootstrap/modules/InfrastructureModule.h"
#include "app/bootstrap/modules/UiModule.h"
#include "app/bootstrap/modules/ViewModelsModule.h"
#include "domain/ports/inbound/IVideoSource.h"
#include "domain/ports/inbound/IAngleCalculator.h"
#include "application/ports/outbound/settings/IVideoSourceGridSettingsRepository.h"
#include "viewmodels/video/VideoSourceGridViewModel.h"
#include "viewmodels/video/VideoSourceViewModel.h"
#include "viewmodels/settings/VideoSourceGridSettingsViewModel.h"
#include "viewmodels/MainWindowViewModel.h"
#include "ui/widgets/QtMainWindow.h"

AppBootstrap::AppBootstrap(std::string configDirectory)
    : configDirectory_(std::move(configDirectory))
    , lifecycle_(domain::common::ProcessLifecycleState::Idle)
{
}

AppBootstrap::~AppBootstrap() {
}

void AppBootstrap::initialize() {
    auto loggerConfig = InfrastructureModule::loadLoggerConfig(configDirectory_);
    logger_ = InfrastructureModule::createLogger(loggerConfig);

    processRunner_ = InfrastructureModule::createProcessRunner(lifecycle_);
    lifecycle_.markForward();

    infra::camera::CameraPorts cameraPorts{
        .logger = *logger_,
        .clock = lifecycle_.clock(),
    };

    videoSources_ = InfrastructureModule::createVideoSources(configDirectory_, "cameras.ini", cameraPorts);

    const auto anglemeterConfig = DomainModule::loadAnglemeterConfig(configDirectory_);
    castAnglemeter_ = DomainModule::createAnglemeter(*logger_, anglemeterConfig);
    angleInteractor_ = DomainModule::createAngleFromVideoInteractor(*logger_, *castAnglemeter_);
    angleInteractor_->start();

    std::vector<domain::ports::IVideoSource*> videoSources{};
    for (const auto& videoSource : videoSources_) {
        videoSources.push_back(videoSource.get());
    }

    sourceViewModels_ = ViewModelsModule::createVideoSourceViewModels(videoSources);
    gridViewModel_ = ViewModelsModule::createGridViewModel(sourceViewModels_);

    settingsRepository_ = InfrastructureModule::createSettingsRepository(configDirectory_);
    gridService_ = ViewModelsModule::createGridService(videoSources);
    settingsUseCase_ = ViewModelsModule::createSettingsUseCase(
        *logger_,
        *settingsRepository_,
        *gridService_,
        *sourceViewModels_.front());
    settingsViewModel_ = ViewModelsModule::createSettingsViewModel(*settingsUseCase_);
    mainWindowViewModel_ = ViewModelsModule::createMainWindowViewModel(*gridViewModel_, *settingsViewModel_);

    const auto windowConfig = UiModule::loadWindowConfig(configDirectory_);
    mainWindow_ = UiModule::createMainWindow(*mainWindowViewModel_, windowConfig);

    if (!mainWindow_) {
        throw std::runtime_error("Failed to create main window");
    }
}

ui::QtMainWindow& AppBootstrap::mainWindow() {
    if (!mainWindow_) {
        throw std::runtime_error("AppBootstrap::initialize must be called before mainWindow()");
    }

    return *mainWindow_;
}
