#include "ApplicationBootstrap.h"

#include "application/models/logging/LogSource.h"
#include "application/orchestrators/VideoSourceManager.h"
#include "application/ports/outbound/catalogs/IDisplacementCatalog.h"
#include "application/ports/outbound/catalogs/IGaugeCatalog.h"
#include "application/ports/outbound/catalogs/IPrecisionCatalog.h"
#include "application/ports/outbound/catalogs/IPressureUnitCatalog.h"
#include "application/ports/outbound/catalogs/IPrinterCatalog.h"
#include "application/ports/outbound/logging/ILogSourcesStorage.h"
#include "application/ports/outbound/settings/IInfoSettingsStorage.h"
#include "application/ports/outbound/video/IVideoAngleSourcesStorage.h"
#include "domain/ports/inbound/IAngleCalculator.h"
#include "domain/ports/inbound/IAngleSource.h"
#include "domain/ports/inbound/ICalibrationCalculator.h"
#include "domain/ports/inbound/IPressureSource.h"
#include "domain/ports/inbound/IVideoSource.h"
#include "domain/ports/inbound/IProcessLifecycle.h"
#include "infrastructure/process/ProcessRunner.h"
#include "domain/ports/outbound/IResultStore.h"
#include "domain/ports/inbound/IPressureActuator.h"
#include "domain/ports/outbound/IClock.h"
#include "application/services/calibration/CalibrationPolicyService.h"
#include "app/bootstrap/assemblers/CatalogsAssembler.h"
#include "app/bootstrap/assemblers/RuntimeAssembler.h"
#include "app/bootstrap/assemblers/SourcesAssembler.h"
#include "infrastructure/logging/ConsoleLogger.h"
#include "infrastructure/logging/FileLogger.h"
#include "infrastructure/logging/NamedMultiLogger.h"
#include "infrastructure/storage/LogSourcesStorage.h"

using namespace domain::ports;
using namespace application::models;

namespace {
domain::ports::ILogger& createLoggerAdapter(void* context, const std::string& name) {
    return static_cast<ApplicationBootstrap*>(context)->createLogger(name);
}
}

ApplicationBootstrap::ApplicationBootstrap(const std::string &setup_dir, const std::string &catalogs_dir, const std::string &logs_dir)
    : setup_dir_(setup_dir), catalogs_dir_(catalogs_dir), logs_dir_(logs_dir)
{
}

ApplicationBootstrap::~ApplicationBootstrap() = default;

void ApplicationBootstrap::initialize() {
    log_sources_storage = std::make_unique<infra::storage::LogSourcesStorage>();

    auto runtime = app::bootstrap::assembleRuntime();
    process_lifecycle = std::move(runtime.process_lifecycle);
    session_clock = runtime.session_clock;
    uptime_clock = std::move(runtime.uptime_clock);

    auto catalogs = app::bootstrap::assembleCatalogs(catalogs_dir_, createLoggerAdapter, this);
    displacement_catalog = std::move(catalogs.displacement_catalog);
    gauge_catalog = std::move(catalogs.gauge_catalog);
    precision_catalog = std::move(catalogs.precision_catalog);
    pressure_unit_catalog = std::move(catalogs.pressure_unit_catalog);
    printer_catalog = std::move(catalogs.printer_catalog);
    info_settings_storage = std::move(catalogs.info_settings_storage);

    anglemeter = app::bootstrap::assembleAnglemeter(setup_dir_, createLoggerAdapter, this);
    calibrator = app::bootstrap::assembleCalibrator(setup_dir_, createLoggerAdapter, this);
    calibration_policy = std::make_unique<application::services::CalibrationPolicyService>(
        *calibrator,
        createLogger("CalibrationPolicyService")
    );

    auto sources = app::bootstrap::assembleSources(setup_dir_, *session_clock, *anglemeter,  createLoggerAdapter, this);
    video_sources = std::move(sources.video_sources);
    angle_sources = std::move(sources.angle_sources);
    videoangle_sources_storage = std::move(sources.videoangle_sources_storage);
    video_source_manager = std::move(sources.video_source_manager);
    pressure_source = std::move(sources.pressure_source);
}

ILogger & ApplicationBootstrap::createLogger(const std::string &logger_name) {
    auto multi_logger = std::make_unique<infra::logging::NamedMultiLogger>(*uptime_clock, logger_name);

    auto file_logger = std::make_unique<infra::logging::FileLogger>(logs_dir_ + "/" + logger_name + ".log");
    multi_logger->addLogger(*file_logger);

    auto console_logger = std::make_unique<infra::logging::ConsoleLogger>();
    multi_logger->addLogger(*console_logger);

    infra::logging::NamedMultiLogger &multi_logger_ref = *multi_logger;

    loggers.emplace_back(std::move(file_logger));
    loggers.emplace_back(std::move(console_logger));
    loggers.emplace_back(std::move(multi_logger));

    LogSource log_source {
        .name = logger_name,
        .source = &multi_logger_ref
    };

    dynamic_cast<infra::storage::LogSourcesStorage*>(log_sources_storage.get())->addLogSource(log_source);

    return multi_logger_ref;
}

