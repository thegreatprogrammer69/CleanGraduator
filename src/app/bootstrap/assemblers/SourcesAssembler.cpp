#include "SourcesAssembler.h"

#include "application/ports/outbound/logging/ILoggerFactory.h"
#include "application/models/video/VideoSource.h"
#include "application/orchestrators/AngleFromVideoInteractor.h"
#include "application/orchestrators/VideoSourceManager.h"
#include "infrastructure/factory/AngleCalculatorFactory.h"
#include "infrastructure/factory/CalibrationCalculatorFactory.h"
#include "infrastructure/factory/PressureSourceFactory.h"
#include "infrastructure/factory/VideoSourceFactory.h"
#include "infrastructure/storage/VideoAngleSourcesStorage.h"

using namespace app::bootstrap;

namespace {
struct LoggerFactory final : application::ports::ILoggerFactory {
    LoggerFactory(LoggerFactoryFn factory, void* context): factory_(factory), context_(context) {}
    domain::ports::ILogger* create() override {
        cams++;
        return &factory_(context_, "IVideoSource_" + std::to_string(cams));
    }
    int cams = -1;
    LoggerFactoryFn factory_;
    void* context_;
};
}

std::unique_ptr<domain::ports::IAngleCalculator> app::bootstrap::assembleAnglemeter(const std::string &setup_dir, LoggerFactoryFn logger_factory, void *logger_context) {
    infra::calc::AnglemeterPorts ports {
        .logger = logger_factory(logger_context, "IAngleCalculator"),
    };
    infra::repo::AngleCalculatorFactory factory(setup_dir + "/anglemeter.ini", ports);
    return factory.load();
}

std::unique_ptr<domain::ports::ICalibrationCalculator> app::bootstrap::assembleCalibrator(const std::string &setup_dir, LoggerFactoryFn logger_factory, void *logger_context) {
    infra::calc::CalibrationCalculatorPorts ports {
        .logger = logger_factory(logger_context, "ICalibrationCalculator"),
    };
    infra::repo::CalibrationCalculatorFactory factory(setup_dir + "/calibrator.ini", ports);
    return factory.load();
}

SourcesAssembly app::bootstrap::assembleSources(
    const std::string &setup_dir,
    domain::ports::IClock &session_clock,
    domain::ports::IAngleCalculator &anglemeter,
        LoggerFactoryFn logger_factory,
    void *logger_context
) {
    SourcesAssembly assembled;

    LoggerFactory loggerFactory(logger_factory, logger_context);
    infra::repo::VideoSourceFactory video_factory(setup_dir + "/cameras.ini", session_clock, loggerFactory);
    assembled.video_sources = video_factory.load();

    int idx = 1;
    for (const auto& video_source : assembled.video_sources) {
        application::orchestrators::AngleFromVideoInteractorPorts ports {
            .logger = logger_factory(logger_context, "IAngleSource_" + std::to_string(idx)),
            .anglemeter = anglemeter,
            .video_source = *video_source,
        };
        assembled.angle_sources.emplace_back(std::make_unique<application::orchestrators::AngleFromVideoInteractor>(ports));
        idx++;
    }

    auto storage = std::make_unique<infra::storage::VideoAngleSourcesStorage>();
    for (int i = 0; i < assembled.video_sources.size(); i++) {
        storage->add(application::models::VideoAngleSource {
            .id = i + 1,
            .angle_source = *assembled.angle_sources[i],
            .video_source = *assembled.video_sources[i],
        });
    }
    assembled.videoangle_sources_storage = std::move(storage);
    assembled.video_source_manager = std::make_unique<application::orchestrators::VideoSourceManager>(*assembled.videoangle_sources_storage);

    infra::pressure::PressureSourcePorts pressure_ports {
        .logger = logger_factory(logger_context, "IPressureSource"),
        .clock = session_clock,
    };
    infra::repo::PressureSourceFactory pressure_factory(setup_dir + "/pressure_source.ini", pressure_ports);
    assembled.pressure_source = pressure_factory.load();

    return assembled;
}
