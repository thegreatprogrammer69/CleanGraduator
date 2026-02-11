#include "AppBootstrap.h"

#include <stdexcept>
#include <vector>

#include "application/interactors/AngleFromVideoInteractor.h"
#include "app/bootstrap/modules/DomainModule.h"
#include "app/bootstrap/modules/InfrastructureModule.h"
#include "app/bootstrap/modules/UiModule.h"
#include "app/bootstrap/modules/ViewModelsModule.h"
#include "domain/ports/inbound/IVideoSource.h"
#include "ui/widgets/QtMainWindow.h"

AppBootstrap::AppBootstrap(std::string configDirectory)
    : configDirectory_(std::move(configDirectory))
    , lifecycle_(domain::common::ProcessLifecycleState::Idle)
{
}

void AppBootstrap::initialize() {
    auto loggerConfig = InfrastructureModule::loadLoggerConfig(configDirectory_);
    logger_ = InfrastructureModule::createLogger(loggerConfig);

    processRunner_ = InfrastructureModule::createProcessRunner(lifecycle_);
    lifecycle_.markForward();

    const auto primaryCameraConfig = InfrastructureModule::loadCameraConfig(configDirectory_, "camera_primary.ini");
    const auto secondaryCameraConfig = InfrastructureModule::loadCameraConfig(configDirectory_, "camera_secondary.ini");

    infra::camera::CameraPorts cameraPorts{
        .logger = *logger_,
        .clock = lifecycle_.clock(),
    };

    primaryCamera_ = InfrastructureModule::createCamera(primaryCameraConfig, cameraPorts);
    secondaryCamera_ = InfrastructureModule::createCamera(secondaryCameraConfig, cameraPorts);

    const auto anglemeterConfig = DomainModule::loadAnglemeterConfig(configDirectory_);
    castAnglemeter_ = DomainModule::createAnglemeter(*logger_, anglemeterConfig);
    angleInteractor_ = DomainModule::createAngleFromVideoInteractor(*logger_, *castAnglemeter_);
    angleInteractor_->start();

    std::vector<domain::ports::IVideoSource*> sources{primaryCamera_.get(), secondaryCamera_.get()};
    sourceViewModels_ = ViewModelsModule::createVideoSourceViewModels(sources);
    gridViewModel_ = ViewModelsModule::createGridViewModel(sourceViewModels_);

    settingsRepository_ = InfrastructureModule::createSettingsRepository(configDirectory_);
    gridService_ = ViewModelsModule::createGridService(sources);
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
