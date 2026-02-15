#include "ApplicationBootstrap.h"

#include <stdexcept>
#include <vector>

#include "application/orchestrators/VideoSourceManager.h"
#include "application/ports/outbound/logging/ILoggerFactory.h"
#include "infrastructure/calculation/angle/CastAnglemeter.h"
#include "infrastructure/calculation/calibration/CalibrationCalculatorPorts.h"
#include "infrastructure/catalogs/FileDisplacementCatalog.h"
#include "infrastructure/catalogs/FileGaugeCatalog.h"
#include "infrastructure/catalogs/FilePrecisionCatalog.h"
#include "infrastructure/catalogs/FilePressureUnitCatalog.h"
#include "infrastructure/catalogs/FilePrinterCatalog.h"
#include "infrastructure/clock/SessionClock.h"
#include "infrastructure/clock/UptimeClock.h"
#include "infrastructure/factory/AngleCalculatorFactory.h"
#include "infrastructure/factory/CalibrationCalculatorFactory.h"
#include "infrastructure/factory/PressureSourceFactory.h"
#include "infrastructure/factory/VideoSourceFactory.h"
#include "infrastructure/logging/ConsoleLogger.h"
#include "infrastructure/logging/FileLogger.h"
#include "infrastructure/motor/g540/G540LPT.h"
#include "infrastructure/overlay/crosshair/CrosshairVideoOverlay.h"
#include "infrastructure/platform/com/ComPort.h"
#include "infrastructure/pressure/PressureSourceNotifier.h"
#include "infrastructure/pressure/dm5002/PressureSourcePorts.h"
#include "infrastructure/process/ProcessLifecycle.h"
#include "infrastructure/storage/QtSettingsStorage.h"
#include "infrastructure/storage/VideoSourcesStorage.h"
#include "viewmodels/settings/SettingsViewModel.h"

#include "domain/ports/inbound/IVideoSource.h"

using namespace mvvm;
using namespace domain::ports;
using namespace domain::common;
using namespace application::orchestrators;
using namespace application::models;
using namespace application::ports;
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
using namespace infra::storage;


struct LoggerFactory final : ILoggerFactory {
    explicit LoggerFactory(ApplicationBootstrap& app) : app(app) {}
    ~LoggerFactory() override = default;
    ILogger* create() override {
        cams++;
        return &app.createLogger("IVideoSource " + std::to_string(cams));
    }
    int cams = -1;
    ApplicationBootstrap& app;
};


// AppBootstrap::AppBootstrap(const std::string &setup_dir, const std::string &catalogs_dir, const std::string &logs_dir)
//     : setup_dir_(setup_dir), catalogs_dir_(catalogs_dir), logs_dir_(logs_dir)
// {
// }
//
// AppBootstrap::~AppBootstrap() {
// }
//
//
// ILogger& AppBootstrap::createLogger(const std::string &logger_name) {
//     loggers_.push_back(std::make_unique<ConsoleLogger>());
//     return *loggers_.back();
// }
//
//
// std::unique_ptr<IProcessLifecycle> AppBootstrap::createLifecycle() {
//     return std::make_unique<ProcessLifecycle>();
// }
//
// std::vector<std::unique_ptr<IVideoSource>> AppBootstrap::createVideoSources() {
//     LoggerFactory logger_factory(*this);
//     VideoSourceFactory factory(setup_dir_ + "/cameras.ini",process_lifecycle_->clock(), logger_factory);
//     return factory.load();
// }
//
// std::unique_ptr<IVideoSourcesStorage> AppBootstrap::createVideoSourceStorage() {
//     std::vector<VideoSource> sources; int id = 1;
//     for (const auto& video_source : video_sources_) {
//         VideoSource source {
//             .id = id,
//             .video_source = *video_source
//         };
//         sources.push_back(source); id++;
//     }
//     return std::make_unique<VideoSourcesStorage>(sources);
// }
//
// std::unique_ptr<VideoSourceManager> AppBootstrap::createVideoSourceManager() {
//     return std::make_unique<VideoSourceManager>(*video_source_storage_);
// }
//
// std::unique_ptr<IAngleCalculator> AppBootstrap::createAnglemeter() {
//     AnglemeterPorts ports {
//         .logger = createLogger("IAngleCalculator"),
//     };
//     AngleCalculatorFactory factory(setup_dir_ + "/anglemeter.ini", ports);
//     return factory.load();
// }
//
// std::vector<std::unique_ptr<AngleFromVideoInteractor>> AppBootstrap::createAngleFromVideoInteractors() {
//     std::vector<std::unique_ptr<AngleFromVideoInteractor>> resut;
//
//     int i = 0;
//     for (const auto& video_source : video_sources_) {
//         AngleFromVideoInteractorPorts ports {
//             .logger = createLogger("AngleFromVideoInteractor_" + std::to_string(i)),
//             .anglemeter = *anglemeter_,
//             .video_source = *video_source
//         };
//         resut.push_back(std::make_unique<AngleFromVideoInteractor>(ports));
//         i++;
//     }
//
//     return resut;
// }
//
// std::unique_ptr<ICalibrationCalculator> AppBootstrap::createCalibrator() {
//     CalibrationCalculatorPorts ports {
//         .logger = createLogger("ICalibrationCalculator"),
//     };
//     CalibrationCalculatorFactory factory(setup_dir_ + "/calibrator.ini", ports);
//     return factory.load();
// }
//
//
// std::unique_ptr<IPressureSource> AppBootstrap::createPressureSource() {
//     PressureSourcePorts ports {
//         .logger = createLogger("IPressureSource"),
//         .clock = process_lifecycle_->clock()
//     };
//     PressureSourceFactory factory(setup_dir_ + "/pressure_source.ini", ports);
//     return factory.load();
// }
//
// // std::unique_ptr<IResultStore> AppBootstrap::createResultStore() {
// //
// //
// // }
//
// std::unique_ptr<IDisplacementCatalog> AppBootstrap::createDisplacementCatalog() {
//     return std::make_unique<FileDisplacementCatalog>(catalogs_dir_ + "/displacements");
// }
//
// std::unique_ptr<IPrinterCatalog> AppBootstrap::createPrinterCatalog() {
//     return std::make_unique<FilePrinterCatalog>(catalogs_dir_ + "/printers");
// }
//
// std::unique_ptr<IPrecisionCatalog> AppBootstrap::createPrecisionCatalog() {
//     return std::make_unique<FilePrecisionCatalog>(catalogs_dir_ + "/precision_classes");
// }
//
// std::unique_ptr<IPressureUnitCatalog> AppBootstrap::createPressureUnitCatalog() {
//     return std::make_unique<FilePressureUnitCatalog>(catalogs_dir_ + "/pressure_units");
// }
//
// std::unique_ptr<IGaugeCatalog> AppBootstrap::createGaugeCatalog() {
//     return std::make_unique<FileGaugeCatalog>(catalogs_dir_ + "/gauges");
// }
//
//
// std::unique_ptr<ISettingsStorage> AppBootstrap::createSettingsStorage() {
//     return std::make_unique<infra::settings::QtSettingsStorage>("CleanGraduator", "CleanGraduator");
// }
//
//
// std::unique_ptr<ProcessRunner> AppBootstrap::createProcessRunner() {
//     return std::make_unique<ProcessRunner>(createLogger("ProcessRunner"), *process_lifecycle_);
// }
//
// std::unique_ptr<OpenSelectedCameras> AppBootstrap::createOpenSelectedCamerasUseCase() {
//     return std::make_unique<OpenSelectedCameras>(*video_source_manager_);
// }
//
// std::vector<std::unique_ptr<VideoSourceViewModel>> AppBootstrap::createVideoSourceViewModels() {
//     std::vector<std::unique_ptr<VideoSourceViewModel>> view_models;
//     for (const auto& video_source : video_sources_) {
//         view_models.push_back(std::make_unique<VideoSourceViewModel>(*video_source));
//     }
//     return view_models;
// }
//
// std::unique_ptr<VideoSourceGridViewModel> AppBootstrap::createVideoSourceGridViewModel() {
//     constexpr int columns = 2;
//     constexpr int rows = 4;
//     constexpr double aspectRatioHW = 4.0 / 3.0;
//
//     VideoSourceGridViewModel::Slots vm_slots;
//     int i = 0;
//     for (const auto& view_model : video_source_view_models_) {
//
//         int row = i % rows;
//         int col = i / rows;
//
//         vm_slots.emplace_back(row, col, *view_model);
//
//         ++i;
//     }
//
//     return std::make_unique<VideoSourceGridViewModel>(
//         vm_slots,
//         rows,
//         columns,
//         aspectRatioHW
//     );
// }
//
// std::unique_ptr<CameraGridSettingsViewModel> AppBootstrap::createCameraGridSettingsViewModel() {
//     CameraGridSettingsViewModelDeps deps {};
//     return std::make_unique<CameraGridSettingsViewModel>();
// }
//
// std::unique_ptr<SettingsViewModel> AppBootstrap::createSettingsViewModel() {
//     SettingsViewModelDeps deps {
//         .camera_grid = *camera_grid_settings_view_model_
//     };
//     return std::make_unique<SettingsViewModel>(deps);
// }
//
// std::unique_ptr<MainWindowViewModel> AppBootstrap::createMainWindowViewModel() {
//     MainWindowViewModelDeps deps {
//         .grid = *video_source_grid_view_model_,
//         .settings = *settings_view_model_
//     };
//     return std::make_unique<MainWindowViewModel>(deps);
// }
//
// std::unique_ptr<ui::QtMainWindow> AppBootstrap::createQtMainWindow() {
//     return std::make_unique<ui::QtMainWindow>(*main_window_view_model_);
// }
//
//
// void AppBootstrap::initialize() {
//
//     process_lifecycle_ = createLifecycle();
//
//     process_runner_ = createProcessRunner();
//
//     video_sources_ = createVideoSources();
//
//     // Алгоритмя вычисления угла и калибровки
//     anglemeter_ = createAnglemeter();
//     calibrator_ = createCalibrator();
//
//     // Перехватывает видео с источника и вычисляет угол, один для каждой камеры
//     angle_from_video_interactors_ = createAngleFromVideoInteractors();
//
//     // Датчик давления
//     pressure_source_ = createPressureSource();
//
//     // Каталоги
//     displacement_catalog_ = createDisplacementCatalog();
//     printer_catalog_ = createPrinterCatalog();
//     precision_catalog_ = createPrecisionCatalog();
//     pressure_unit_catalog_ = createPressureUnitCatalog();
//     gauge_catalog_ = createGaugeCatalog();
//
//
//     // Storages
//     video_source_storage_ = createVideoSourceStorage();
//     settings_storage_ = createSettingsStorage();
//
//     // Orchestrators
//     video_source_manager_ = createVideoSourceManager();
//
//
//     // Use Cases
//     open_selected_cameras_use_case_ = createOpenSelectedCamerasUseCase();
//
//     // ViewModels
//     video_source_view_models_ = createVideoSourceViewModels();
//     video_source_grid_view_model_ = createVideoSourceGridViewModel();
//
//     // Строка номеров камер
//     camera_grid_settings_view_model_ = createCameraGridSettingsViewModel();
//
//     // Страница настроек
//     settings_view_model_ = createSettingsViewModel();
//
//     // Главное окно
//     main_window_view_model_ = createMainWindowViewModel();
//
//     // Widgets
//     main_window_ = createQtMainWindow();
//
// }

// ui::QtMainWindow& AppBootstrap::mainWindow() {
//     if (!main_window_) {
//         throw std::runtime_error("AppBootstrap::initialize must be called before mainWindow()");
//     }
//
//     return *main_window_;
// }

void ApplicationBootstrap::createLifecycle() {
    process_lifecycle = std::make_unique<ProcessLifecycle>();
}

void ApplicationBootstrap::createClock() {
    session_clock = &process_lifecycle->clock();
    uptime_clock = std::make_unique<UptimeClock>();
}

void ApplicationBootstrap::createVideoSources() {
    LoggerFactory logger_factory(*this);
    VideoSourceFactory factory(setup_dir_ + "/cameras.ini",*session_clock, logger_factory);
    video_sources = factory.load();
}

void ApplicationBootstrap::createVideoSourceStorage() {
    auto storage = std::make_unique<VideoSourcesStorage>();

    std::vector<VideoSource> sources; int id = 1;
    for (const auto& video_source : video_sources) {
        const VideoSource source {
            .id = id,
            .video_source = *video_source
        };
        storage->add(source);
        id++;
    }

    video_sources_storage = std::move(storage);
}

void ApplicationBootstrap::createVideoSourceManager() {
    video_source_manager = std::make_unique<VideoSourceManager>(*video_sources_storage);
}

void ApplicationBootstrap::createAnglemeter() {
    const AnglemeterPorts ports {
        .logger = createLogger("IAngleCalculator"),
    };
    AngleCalculatorFactory factory(setup_dir_ + "/anglemeter.ini", ports);
    anglemeter = factory.load();
}

void ApplicationBootstrap::createCalibrator() {
    const CalibrationCalculatorPorts ports {
        .logger = createLogger("ICalibrationCalculator"),
    };
    CalibrationCalculatorFactory factory(setup_dir_ + "/calibrator.ini", ports);
    calibrator = factory.load();
}

void ApplicationBootstrap::createPressureSource() {
    const PressureSourcePorts ports {
        .logger = createLogger("IPressureSource"),
        .clock = *session_clock,
    };
    PressureSourceFactory factory(setup_dir_ + "/pressure_source.ini", ports);
    pressure_source = factory.load();
}

void ApplicationBootstrap::createDisplacementCatalog() {
    FileDisplacementCatalogPorts ports {
        .logger = createLogger("IDisplacementCatalog"),
    };
    displacement_catalog = std::make_unique<FileDisplacementCatalog>(ports, catalogs_dir_ + "/displacements");
}

void ApplicationBootstrap::createPrinterCatalog() {
    FilePrinterCatalogPorts ports {
        .logger = createLogger("IPrinterCatalog"),
    };
    printer_catalog = std::make_unique<FilePrinterCatalog>(ports, catalogs_dir_ + "/printers");
}

void ApplicationBootstrap::createPrecisionCatalog() {
    FilePrecisionCatalogPorts ports {
        .logger = createLogger("IPrecisionCatalog"),
    };
    precision_catalog = std::make_unique<FilePrecisionCatalog>(ports, catalogs_dir_ + "/precision_classes");
}

void ApplicationBootstrap::createPressureUnitCatalog() {
    FilePressureUnitCatalogPorts ports {
        .logger = createLogger("IPressureUnitCatalog"),
    };
    pressure_unit_catalog = std::make_unique<FilePressureUnitCatalog>(ports, catalogs_dir_ + "/pressure_units");
}

void ApplicationBootstrap::createGaugeCatalog() {
    FileGaugeCatalogPorts ports {
        .logger = createLogger("IGaugeCatalog"),
    };
    gauge_catalog = std::make_unique<FileGaugeCatalog>(ports, catalogs_dir_ + "/gauges");
}

void ApplicationBootstrap::createSettingsStorage() {
}

void ApplicationBootstrap::createProcessRunner() {
}
