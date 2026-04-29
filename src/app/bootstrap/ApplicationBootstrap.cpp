#include "ApplicationBootstrap.h"

#include <stdexcept>
#include <filesystem>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <vector>
#include <QSettings>

#include "application/orchestrators/calibration/result/CalibrationResultBuilder.h"
#include "application/orchestrators/calibration/result/CalibrationResultValidationSource.h"
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
#include "infrastructure/logging/FileLoggingControl.h"
#include "infrastructure/logging/NamedMultiLogger.h"
#include "infrastructure/overlay/crosshair/CrosshairVideoOverlay.h"
#include "infrastructure/platform/com/ComPort.h"
#include "infrastructure/pressure/PressureSourceNotifier.h"
#include "infrastructure/pressure/PressureSourcePorts.h"
#include "infrastructure/storage/QtInfoSettingsStorage.h"
#include "infrastructure/storage/VideoAngleSourcesStorage.h"
#include "infrastructure/storage/LogSourcesStorage.h"
#include "viewmodels/settings/SettingsViewModel.h"
#include "domain/ports/video/IVideoSource.h"
#include "shared/ini/IniFile.h"

#include "infrastructure/angle/from_video/AngleSourceFromVideo.h"
#include "infrastructure/calibration/recording/in_memory/InMemoryCalibrationRecorder.h"
#include "infrastructure/calibration/strats/CalibrationStrategyPorts.h"
#include "infrastructure/calibration/strats/stand4/Stand4CalibrationStrategy.h"
#include "infrastructure/calibration/strats/stand4/Stand4CalibrationStrategyConfig.h"
#include "infrastructure/factory/AngleSourceFactory.h"
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
using namespace infra::motor;
using namespace infra::overlay;
using namespace infra::platform;
using namespace infra::repo;
using namespace infra::storage;

namespace {
    constexpr const char* kInfoSettingsGroup = "InfoSettings";
    constexpr const char* kFileLoggingKey = "file_logging_enabled";

    std::string makeSessionLogsDirectory(const std::string& base_logs_dir) {
        const auto now = std::chrono::system_clock::now();
        const std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        const std::tm tm = *std::localtime(&now_time);

        std::ostringstream stream;
        stream << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S");

        return (std::filesystem::path(base_logs_dir) / stream.str()).string();
    }

    class ConditionalFileLogger final : public ILogger {
    public:
        ConditionalFileLogger(ILogger& logger, application::ports::IFileLoggingControl& control)
            : logger_(logger)
            , control_(control) {
        }

        void info(const std::string& msg) override {
            if (control_.isFileLoggingEnabled()) {
                logger_.info(msg);
            }
        }

        void warn(const std::string& msg) override {
            if (control_.isFileLoggingEnabled()) {
                logger_.warn(msg);
            }
        }

        void error(const std::string& msg) override {
            if (control_.isFileLoggingEnabled()) {
                logger_.error(msg);
            }
        }

    private:
        ILogger& logger_;
        application::ports::IFileLoggingControl& control_;
    };

    class PrefixedLogger final : public ILogger {
    public:
        PrefixedLogger(ILogger& logger, std::string name)
            : logger_(logger)
            , prefix_("[" + std::move(name) + "] ") {
        }

        void info(const std::string& msg) override {
            logger_.info(prefix_ + msg);
        }

        void warn(const std::string& msg) override {
            logger_.warn(prefix_ + msg);
        }

        void error(const std::string& msg) override {
            logger_.error(prefix_ + msg);
        }

    private:
        ILogger& logger_;
        std::string prefix_;
    };
}


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
    createFileLoggingControl();
    createSessionLogsDirectory();

    createSessionClock();
    createClock();

    createDisplacementCatalog();
    createGaugeCatalog();
    createPrecisionCatalog();
    createPrinterCatalog();
    createPressureUnitCatalog();

    createAnglemeter();

    createVideoSources();
    createAngleSources();
    createVideoSourcesStorage();
    createVideoSourcesManager();

    createPressureSource();
    createMotorDriver();

    createCalibrationStrategy();
    createCalibrationRecorder();
    createCalibrationCalculator();
    createCalibrationResultSource();

    createInfoSettingsStorage();
    createCalibrationResultValidationSource();
}

ILogger & ApplicationBootstrap::createLogger(const std::string &logger_name) {
    auto multi_logger = std::make_unique<NamedMultiLogger>(*uptime_clock, logger_name);

    if (!session_file_logger_) {
        auto session_file_logger = std::make_unique<FileLogger>(session_logs_dir_ + "/session.log");
        session_file_logger_ = session_file_logger.get();
        loggers.emplace_back(std::move(session_file_logger));
    }

    auto conditional_file_logger = std::make_unique<ConditionalFileLogger>(*session_file_logger_, *file_logging_control);
    auto prefixed_file_logger = std::make_unique<PrefixedLogger>(*conditional_file_logger, logger_name);
    multi_logger->addLogger(*prefixed_file_logger);

    auto console_logger = std::make_unique<ConsoleLogger>();
    multi_logger->addLogger(*console_logger);

    NamedMultiLogger &multi_logger_ref = *multi_logger;

    loggers.emplace_back(std::move(conditional_file_logger));
    loggers.emplace_back(std::move(prefixed_file_logger));
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

void ApplicationBootstrap::createFileLoggingControl() {
    QSettings settings("CleanGraduator", "CleanGraduator");
    settings.beginGroup(kInfoSettingsGroup);
    const bool enabled = settings.value(kFileLoggingKey, true).toBool();
    settings.endGroup();

    file_logging_control = std::make_unique<FileLoggingControl>(enabled);
}

void ApplicationBootstrap::createSessionLogsDirectory() {
    session_logs_dir_ = makeSessionLogsDirectory(logs_dir_);
    std::filesystem::create_directories(session_logs_dir_);
}

void ApplicationBootstrap::createSessionClock() {
    session_clock = std::make_unique<SessionClock>();
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
        infra::angle::AngleSourcePorts ports{
            createLogger("IAngleSource_" + std::to_string(idx)),
            *anglemeter,
            *video_source
        };

        AngleSourceFactory factory(setup_dir_ + "/angle_sources.ini", ports);
        angle_sources.emplace_back(factory.load(SourceId{idx}));

        ++idx;
    }
}

void ApplicationBootstrap::createVideoSourcesStorage() {
    auto storage = std::make_unique<VideoAngleSourcesStorage>();

    for (int idx = 0; idx < video_sources.size(); ++idx) {
        VideoAngleSource source{
             SourceId{idx + 1},
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

}

void ApplicationBootstrap::createCalibrationStrategy() {
    infra::calib::CalibrationStrategyPorts ports {
        createLogger("Stand4CalibrationStrategy")
    };

    infra::calib::stand4::Stand4CalibrationStrategyConfig config {};
    shared::ini::IniFile motor_ini;
    if (motor_ini.load(setup_dir_ + "/motor.ini") && motor_ini.hasSection("motor")) {
        const auto section = motor_ini["motor"];
        config.max_motor_frequency_hz = section.getInt("max_freq_hz", config.max_motor_frequency_hz);
    }

    calibration_strategy = std::make_unique<infra::calib::stand4::Stand4CalibrationStrategy>(ports, config);
}

void ApplicationBootstrap::createCalibrationRecorder() {
    infra::calib::CalibrationRecorderPorts ports {
        createLogger("InMemoryCalibrationRecorder")
    };
    calibration_recorder = std::make_unique<infra::calib::InMemoryCalibrationRecorder>(ports);
}

void ApplicationBootstrap::createCalibrationCalculator() {
    CalibrationCalculatorPorts ports{
        createLogger("CalibrationCalculator")
    };

    CalibrationCalculatorFactory factory(
        setup_dir_ + "/calibrator.ini",
        ports
    );

    calibration_calculator = factory.load();
}

void ApplicationBootstrap::createCalibrationResultSource() {
    CalibrationResultBuilderPorts ports {
        createLogger("CalibrationResultBuilder"),
        *calibration_calculator,
        *calibration_recorder
    };
    calibration_result_source = std::make_unique<CalibrationResultBuilder>(ports);
}


void ApplicationBootstrap::createCalibrationResultValidationSource() {
    CalibrationResultValidationSourceDeps deps{
        createLogger("CalibrationResultValidationSource"),
        *calibration_result_source,
        *info_settings_storage,
        *precision_catalog
    };
    calibration_result_validation_source = std::make_unique<CalibrationResultValidationSource>(deps);
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
