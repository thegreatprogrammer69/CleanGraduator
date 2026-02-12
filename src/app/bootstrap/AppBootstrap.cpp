#include "AppBootstrap.h"

#include <stdexcept>
#include <vector>

#include "application/ports/outbound/ILoggerFactory.h"
#include "infrastructure/calculation/angle/CastAnglemeter.h"
#include "infrastructure/catalogs/FilePrinterCatalog.h"
#include "infrastructure/factory/AngleCalculatorFactory.h"
#include "infrastructure/factory/VideoSourceFactory.h"
#include "infrastructure/logging/ConsoleLogger.h"
#include "infrastructure/logging/FileLogger.h"
#include "infrastructure/motor/g540/G540LPT.h"
#include "infrastructure/overlay/crosshair/CrosshairVideoOverlay.h"
#include "infrastructure/platform/com/ComPort.h"
#include "infrastructure/pressure/PressureSourceNotifier.h"
#include "infrastructure/process/ProcessLifecycle.h"
#include "modules/DomainModule.h"

using namespace domain::ports;
using namespace domain::common;
using namespace application::orchestrators;
using namespace application::models;
using namespace application::ports;
using namespace application::usecase;
using namespace infra::clock;
using namespace infra::process;
using namespace infra::camera;
using namespace infra::logging;
using namespace infra::calc;
using namespace infra::catalogs;
using namespace infra::pressure;
using namespace infra::camera;
using namespace infra::hardware;
using namespace infra::overlay;
using namespace infra::platform;
using namespace infra::repo;


struct LoggerFactory final : ILoggerFactory {
    explicit LoggerFactory(AppBootstrap& app) : app(app) {}
    ~LoggerFactory() override = default;
    ILogger* create() override {
        cams++;
        return &app.createLogger("Camera " + std::to_string(cams));
    }
    int cams = 0;
    AppBootstrap& app;
};


AppBootstrap::AppBootstrap(const std::string &setup_dir, const std::string &catalogs_dir, const std::string &logs_dir)
    : setup_dir_(setup_dir), catalogs_dir_(catalogs_dir), logs_dir_(logs_dir)
{
}

AppBootstrap::~AppBootstrap() {
}


ILogger& AppBootstrap::createLogger(const std::string &logger_name) {
    loggers_.push_back(std::make_unique<ConsoleLogger>());
    return *loggers_.back();
}


std::unique_ptr<IProcessLifecycle> AppBootstrap::createLifecycle() {
    return std::make_unique<ProcessLifecycle>();
}

std::vector<std::unique_ptr<IVideoSource>> AppBootstrap::createVideoSources() {
    LoggerFactory logger_factory(*this);
    VideoSourceFactory factory(setup_dir_ ,process_lifecycle_->clock(), logger_factory);
    return factory.load();
}

std::unique_ptr<IAngleCalculator> AppBootstrap::createAnglemeter() {
    AnglemeterPorts ports {
        .logger = createLogger("Anglemeter"),
    };
    AngleCalculatorFactory factory(setup_dir_, ports);
    return factory.load();
}

std::vector<std::unique_ptr<AngleFromVideoInteractor>> AppBootstrap::createAngleFromVideoInteractor() {
    std::vector<std::unique_ptr<AngleFromVideoInteractor>> resut;

    int i = 0;
    for (const auto& video_source : video_sources_) {
        i++;
        AngleFromVideoInteractorPorts ports {
            .logger = createLogger("AngleFromVideoInteractor " + std::to_string(i)),
            .anglemeter = *anglemeter_,
            .video_source = *video_source
        };
        resut.push_back(std::make_unique<AngleFromVideoInteractor>(ports));
    }

    return resut;
}

void AppBootstrap::initialize() {

    process_lifecycle_ = createLifecycle();

    video_sources_ = createVideoSources();

    anglemeter_ = createAnglemeter();

    angle_from_video_interactors_ = createAngleFromVideoInteractor();



    // std::vector<IVideoSource*> videoSources{};
    // for (const auto& videoSource : videoSources_) {
    //     videoSources.push_back(videoSource.get());
    // }
    //
    // sourceViewModels_ = ViewModelsModule::createVideoSourceViewModels(videoSources);
    // gridViewModel_ = ViewModelsModule::createGridViewModel(sourceViewModels_);
    //
    //
    // settingsRepository_ = InfrastructureModule::createSettingsRepository(configDirectory_);
    //
    // gridSettingsUseCase_ = ViewModelsModule::createCameraGridSettingsUseCase(*logger_,*gridService_);
    // gridSettingsViewModel_ = ViewModelsModule::createCameraGridSettingsViewModel(*gridSettingsUseCase_);
    //
    // mainWindowViewModel_ = ViewModelsModule::createMainWindowViewModel(*gridViewModel_, *gridSettingsViewModel_);
    // const auto windowConfig = UiModule::loadWindowConfig(configDirectory_);
    // mainWindow_ = UiModule::createMainWindow(*mainWindowViewModel_, windowConfig);
    //
    // if (!mainWindow_) {
    //     throw std::runtime_error("Failed to create main window");
    // }
}

ui::QtMainWindow& AppBootstrap::mainWindow() {
    if (!mainWindow_) {
        throw std::runtime_error("AppBootstrap::initialize must be called before mainWindow()");
    }

    return *mainWindow_;
}