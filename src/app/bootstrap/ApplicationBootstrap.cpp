#include "ApplicationBootstrap.h"

#include <stdexcept>
#include <vector>

#include "application/orchestrators/angle/AngleFromVideoInteractor.h"
#include "application/orchestrators/video/VideoSourceManager.h"
#include "application/ports/logging/ILoggerFactory.h"
#include "infrastructure/calculation/angle/CastAnglemeter.h"
#include "infrastructure/calculation/calibration/CalibrationCalculatorPorts.h"
#include "infrastructure/catalogs/FileDisplacementCatalog.h"
#include "infrastructure/catalogs/FileGaugeCatalog.h"
#include "infrastructure/catalogs/FileGaugePrecisionCatalog.h"
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
#include "infrastructure/motion/g540/as_lpt/G540LptMotorDriver.h"
#include "infrastructure/overlay/crosshair/CrosshairVideoOverlay.h"
#include "infrastructure/platform/com/ComPort.h"
#include "infrastructure/pressure/PressureSourceNotifier.h"
#include "infrastructure/pressure/PressureSourcePorts.h"
#include "infrastructure/storage/QtInfoSettingsStorage.h"
#include "infrastructure/storage/VideoAngleSourcesStorage.h"
#include "infrastructure/storage/LogSourcesStorage.h"
#include "viewmodels/settings/SettingsViewModel.h"
#include "domain/ports/video/IVideoSource.h"

#include "domain/ports/calibration/result/IResultStore.h"
#include "infrastructure/factory/MotorDriverFactory.h"

using namespace mvvm;
using namespace domain::ports;
using namespace domain::common;
using namespace application::orchestrators;
using namespace application::models;
using namespace application::ports;
using namespace infra::clock;
using namespace infra::logging;
using namespace infra::calc;
using namespace infra::catalogs;
using namespace infra::pressure;
using namespace infra::motors;
using namespace infra::overlay;
using namespace infra::platform;
using namespace infra::repo;
using namespace infra::storage;
using namespace infra::lifecycle;


struct LoggerFactory final : ILoggerFactory {
    explicit LoggerFactory(ApplicationBootstrap& app) : app(app) {}
    ~LoggerFactory() override = default;
    ILogger* create() override {
        cams++;
        return &app.createLogger("IVideoSource_" + std::to_string(cams));
    }
    int cams = 0;
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
    createMotorDriver();

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


    LogSource log_source;
    log_source.name = logger_name;
    log_source.source = &multi_logger_ref;

    dynamic_cast<LogSourcesStorage*>(log_sources_storage.get())->addLogSource(log_source);

    return multi_logger_ref;
}

void ApplicationBootstrap::createLogSourcesStorage() {
    log_sources_storage = std::make_unique<LogSourcesStorage>();
}

void ApplicationBootstrap::createLifecycle() {
    auto* lifecycle = new CalibrationLifecycle();
    session_clock = &lifecycle->sessionClock();
    calibration_lifecycle = std::unique_ptr<CalibrationLifecycle>(lifecycle);
}

void ApplicationBootstrap::createClock() {

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
        AngleFromVideoInteractorPorts ports{
            createLogger("IAngleSource_" + std::to_string(idx)),
            *anglemeter,
            *video_source
        };

        angle_sources.emplace_back(
            std::make_unique<AngleFromVideoInteractor>(AngleSourceId{idx}, ports)
        );

        ++idx;
    }
}

void ApplicationBootstrap::createVideoSourcesStorage() {
    auto storage = std::make_unique<VideoAngleSourcesStorage>();

    for (int idx = 0; idx < video_sources.size(); ++idx) {
        VideoAngleSource source{
             AngleSourceId{idx + 1},
            *angle_sources[idx],
            *video_sources[idx]
        };

        storage->add(source);
    }

    videoangle_sources_storage = std::move(storage);
}

void ApplicationBootstrap::createVideoSourcesManager() {
    video_source_manager =
        std::make_unique<VideoSourceManager>(
            *videoangle_sources_storage
        );
}

void ApplicationBootstrap::createAnglemeter() {
    AnglemeterPorts ports{
        createLogger("IAngleCalculator")
    };

    AngleCalculatorFactory factory(
        setup_dir_ + "/anglemeter.ini",
        ports
    );

    anglemeter = factory.load();
}

void ApplicationBootstrap::createCalibrator() {
    CalibrationCalculatorPorts ports{
        createLogger("ICalibrationCalculator")
    };

    CalibrationCalculatorFactory factory(
        setup_dir_ + "/calibrator.ini",
        ports
    );

    // calibrator = factory.load();
}

void ApplicationBootstrap::createPressureSource() {
    PressureSourcePorts ports{
        createLogger("IPressureSource"),
        *session_clock
    };

    PressureSourceFactory factory(
        setup_dir_ + "/pressure_source.ini",
        ports
    );

    pressure_source = factory.load();
}


void ApplicationBootstrap::createMotorDriver() {
    MotorDriverPorts ports{
        createLogger("IMotorDriver")
    };
    MotorDriverFactory factory(setup_dir_ + "/motor.ini", ports);
    motor_driver = factory.load();
    valve_driver = factory.load_valve_driver();

}

void ApplicationBootstrap::createDisplacementCatalog() {
    CatalogPorts ports{
        createLogger("IDisplacementCatalog")
    };

    displacement_catalog =
        std::make_unique<FileDisplacementCatalog>(
            ports,
            catalogs_dir_ + "/displacements"
        );
}

void ApplicationBootstrap::createPrinterCatalog() {
    CatalogPorts ports{
        createLogger("IPrinterCatalog")
    };

    printer_catalog =
        std::make_unique<FilePrinterCatalog>(
            ports,
            catalogs_dir_ + "/printers"
        );
}

void ApplicationBootstrap::createPrecisionCatalog() {
    CatalogPorts ports{
        createLogger("IPrecisionCatalog")
    };

    precision_catalog =
        std::make_unique<FileGaugePrecisionCatalog>(
            ports,
            catalogs_dir_ + "/precision_classes"
        );
}

void ApplicationBootstrap::createPressureUnitCatalog() {
    CatalogPorts ports{
        createLogger("IPressureUnitCatalog")
    };

    pressure_unit_catalog =
        std::make_unique<FilePressureUnitCatalog>(
            ports,
            catalogs_dir_ + "/pressure_units"
        );
}

void ApplicationBootstrap::createGaugeCatalog() {
    CatalogPorts ports{
        createLogger("IGaugeCatalog")
    };

    gauge_catalog =
        std::make_unique<FileGaugeCatalog>(
            ports,
            catalogs_dir_ + "/gauges"
        );
}

void ApplicationBootstrap::createInfoSettingsStorage() {
    QtInfoSettingsStorageCatalogs catalogs{
        *displacement_catalog,
        *gauge_catalog,
        *precision_catalog,
        *pressure_unit_catalog,
        *printer_catalog
    };

    info_settings_storage =
        std::make_unique<QtInfoSettingsStorage>(
            "CleanGraduator",
            "CleanGraduator",
            catalogs
        );
}


void ApplicationBootstrap::createProcessRunner() {

}
