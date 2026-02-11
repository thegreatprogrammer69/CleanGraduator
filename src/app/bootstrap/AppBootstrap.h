#ifndef CLEANGRADUATOR_APPBOOTSTRAP_H
#define CLEANGRADUATOR_APPBOOTSTRAP_H

#include <memory>
#include <string>
#include <vector>

#include "infrastructure/process/ProcessLifecycle.h"

namespace application::interactors {
class AngleFromVideoInteractor;
}

namespace application::services {
class VideoSourceGridService;
}

namespace application::usecases {
class ApplyCameraGridSettings;
}

namespace domain::ports {
struct ILogger;
struct IVideoSource;
}

namespace infrastructure::settings {
class QtCameraGridSettingsRepository;
}

namespace infra::calculation {
class CastAnglemeter;
}

namespace infra::process {
class ProcessRunner;
}

namespace mvvm {
class MainWindowViewModel;
class VideoSourceGridSettingsViewModel;
class VideoSourceGridViewModel;
class VideoSourceViewModel;
}

namespace ui {
class QtMainWindow;
}

class AppBootstrap {
public:
    explicit AppBootstrap(std::string configDirectory);

    void initialize();
    ui::QtMainWindow& mainWindow();

private:
    std::string configDirectory_;

    std::unique_ptr<domain::ports::ILogger> logger_;
    infra::process::ProcessLifecycle lifecycle_;
    std::unique_ptr<infra::process::ProcessRunner> processRunner_;

    std::unique_ptr<domain::ports::IVideoSource> primaryCamera_;
    std::unique_ptr<domain::ports::IVideoSource> secondaryCamera_;

    std::unique_ptr<infra::calculation::CastAnglemeter> castAnglemeter_;
    std::unique_ptr<application::interactors::AngleFromVideoInteractor> angleInteractor_;

    std::vector<std::unique_ptr<mvvm::VideoSourceViewModel>> sourceViewModels_;
    std::unique_ptr<mvvm::VideoSourceGridViewModel> gridViewModel_;

    std::unique_ptr<infrastructure::settings::QtCameraGridSettingsRepository> settingsRepository_;
    std::unique_ptr<application::services::VideoSourceGridService> gridService_;
    std::unique_ptr<application::usecases::ApplyCameraGridSettings> settingsUseCase_;
    std::unique_ptr<mvvm::VideoSourceGridSettingsViewModel> settingsViewModel_;
    std::unique_ptr<mvvm::MainWindowViewModel> mainWindowViewModel_;

    std::unique_ptr<ui::QtMainWindow> mainWindow_;
};

#endif //CLEANGRADUATOR_APPBOOTSTRAP_H
