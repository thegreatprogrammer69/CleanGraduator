#include "ApplicationBootstrap.h"

#include <stdexcept>
#include <vector>

#include "application/orchestrators/AngleFromVideoInteractor.h"
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
#include "infrastructure/logging/NamedMultiLogger.h"
#include "infrastructure/motor/g540/G540LPT.h"
#include "infrastructure/overlay/crosshair/CrosshairVideoOverlay.h"
#include "infrastructure/platform/com/ComPort.h"
#include "infrastructure/pressure/PressureSourceNotifier.h"
#include "infrastructure/pressure/dm5002/PressureSourcePorts.h"
#include "infrastructure/process/ProcessLifecycle.h"
#include "infrastructure/storage/QtInfoSettingsStorage.h"
#include "infrastructure/storage/VideoAngleSourcesStorage.h"
#include "infrastructure/storage/LogSourcesStorage.h"
#include "viewmodels/settings/SettingsViewModel.h"
#include "domain/ports/inbound/IVideoSource.h"
#include "domain/ports/inbound/IPressureActuator.h"
#include "domain/ports/outbound/IResultStore.h"
#include "infrastructure/process/ProcessRunner.h"

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
        return &app.createLogger("IVideoSource_" + std::to_string(cams));
    }
    int cams = -1;
    ApplicationBootstrap& app;
};

ApplicationBootstrap::ApplicationBootstrap(const std::string &setup_dir, const std::string &catalogs_dir, const std::string &logs_dir)
    : setup_dir_(setup_dir), catalogs_dir_(catalogs_dir), logs_dir_(logs_dir)
{
}

ApplicationBootstrap::~ApplicationBootstrap() {

}

void ApplicationBootstrap::initialize() {
    createLogSourcesStorage();

    createLifecycle();
    createClock();

    createDisplacementCatalog();
    createGaugeCatalog();
    createPrecisionCatalog();
    createPrinterCatalog();
    createPressureUnitCatalog();

    createAnglemeter();
    createCalibrator();

    createVideoSources();
    createAngleSources();
    createVideoSourcesStorage();
    createVideoSourcesManager();

    createPressureSource();

    createInfoSettingsStorage();
}

ILogger & ApplicationBootstrap::createLogger(const std::string &logger_name) {
    auto multi_logger = std::make_unique<NamedMultiLogger>(*uptime_clock, logger_name);

    auto file_logger = std::make_unique<FileLogger>(logs_dir_ + "/" + logger_name + ".log");
    multi_logger->addLogger(*file_logger);

    auto console_logger = std::make_unique<ConsoleLogger>();
    multi_logger->addLogger(*console_logger);

    NamedMultiLogger &multi_logger_ref = *multi_logger;

    loggers.emplace_back(std::move(file_logger));
    loggers.emplace_back(std::move(console_logger));
    loggers.emplace_back(std::move(multi_logger));


    LogSource log_source {
        .name = logger_name,
        .source = &multi_logger_ref
    };

    dynamic_cast<LogSourcesStorage*>(log_sources_storage.get())->addLogSource(log_source);

    return multi_logger_ref;
}

void ApplicationBootstrap::createLogSourcesStorage() {
    log_sources_storage = std::make_unique<LogSourcesStorage>();
}

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

// В случае кластера из RepkaPI можно сделать разветвление в цикле for
void ApplicationBootstrap::createAngleSources() {
    int idx = 1;
    for (const auto& video_source : video_sources) {
        AngleFromVideoInteractorPorts ports {
            .logger = createLogger("IAngleSource_" + std::to_string(idx)),
            .anglemeter = *anglemeter,
            .video_source = *video_source,
        };
        auto angle_source = std::make_unique<AngleFromVideoInteractor>(ports);
        angle_sources.emplace_back(std::move(angle_source));
        idx++;
    }
}

void ApplicationBootstrap::createVideoSourcesStorage() {
    auto storage = std::make_unique<VideoAngleSourcesStorage>();

    for (int i = 0; i < video_sources.size(); i++) {
        const VideoAngleSource source {
            .id = i + 1,
            .angle_source = *angle_sources[i],
            .video_source = *video_sources[i]
        };
        storage->add(source);
    }

    videoangle_sources_storage = std::move(storage);
}

void ApplicationBootstrap::createVideoSourcesManager() {
    video_source_manager = std::make_unique<VideoSourceManager>(*videoangle_sources_storage);
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

void ApplicationBootstrap::createInfoSettingsStorage() {
    QtInfoSettingsStorageCatalogs catalogs {
        .displacement_catalog = *displacement_catalog,
        .gauge_catalog = *gauge_catalog,
        .precision_catalog = *precision_catalog,
        .pressure_unit_catalog = *pressure_unit_catalog,
        .printer_catalog = *printer_catalog,
    };

    info_settings_storage = std::make_unique<QtInfoSettingsStorage>(
        "CleanGraduator",
        "CleanGraduator",
        catalogs
    );
}

void ApplicationBootstrap::createProcessRunner() {
    process_runner = std::make_unique<ProcessRunner>(createLogger("ProcessRunner"), *process_lifecycle);
}
