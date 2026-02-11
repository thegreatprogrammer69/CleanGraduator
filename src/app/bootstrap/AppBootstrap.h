#ifndef CLEANGRADUATOR_APPBOOTSTRAP_H
#define CLEANGRADUATOR_APPBOOTSTRAP_H

#include <memory>
#include <string>
#include <vector>

#include "infrastructure/process/ProcessLifecycle.h"

namespace application::ports {
    struct IVideoSourceGridSettingsRepository;
}

namespace domain::ports {
    struct IAngleCalculator;
}

namespace application::interactors {
class AngleFromVideoInteractor;
}

namespace application::services {
class CameraGridService;
}

namespace application::usecases {
class ApplyCameraGridSettings;
}

namespace domain::ports {
struct ILogger;
struct IVideoSource;
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
    ~AppBootstrap();
    void initialize();
    ui::QtMainWindow& mainWindow();

private:
    std::string configDirectory_;

    std::unique_ptr<domain::ports::ILogger> logger_;
    infra::process::ProcessLifecycle lifecycle_;
    std::unique_ptr<domain::ports::IProcessLifecycleObserver> processRunner_;

    std::vector<std::unique_ptr<domain::ports::IVideoSource>> videoSources_;

    std::unique_ptr<domain::ports::IAngleCalculator> castAnglemeter_;
    std::unique_ptr<application::interactors::AngleFromVideoInteractor> angleInteractor_;

    std::vector<std::unique_ptr<mvvm::VideoSourceViewModel>> sourceViewModels_;
    std::unique_ptr<mvvm::VideoSourceGridViewModel> gridViewModel_;

    std::unique_ptr<application::ports::IVideoSourceGridSettingsRepository> settingsRepository_;
    std::unique_ptr<application::services::CameraGridService> gridService_;
    std::unique_ptr<application::usecases::ApplyCameraGridSettings> gridSettingsUseCase_;
    std::unique_ptr<mvvm::VideoSourceGridSettingsViewModel> gridSettingsViewModel_;
    std::unique_ptr<mvvm::MainWindowViewModel> mainWindowViewModel_;

    std::unique_ptr<ui::QtMainWindow> mainWindow_;
};

#endif //CLEANGRADUATOR_APPBOOTSTRAP_H
