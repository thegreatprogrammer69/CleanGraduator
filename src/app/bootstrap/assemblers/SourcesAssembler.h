#ifndef CLEANGRADUATOR_SOURCESASSEMBLER_H
#define CLEANGRADUATOR_SOURCESASSEMBLER_H

#include <memory>
#include <string>
#include <vector>

#include "CatalogsAssembler.h"

#include "domain/ports/outbound/IClock.h"
#include "domain/ports/inbound/IAngleCalculator.h"
#include "domain/ports/inbound/ICalibrationCalculator.h"
#include "domain/ports/inbound/IPressureSource.h"
#include "domain/ports/inbound/IVideoSource.h"
#include "domain/ports/inbound/IAngleSource.h"

namespace application::orchestrators { class VideoSourceManager; }
namespace application::ports { struct IVideoAngleSourcesStorage; }

namespace app::bootstrap {

struct SourcesAssembly {
    std::vector<std::unique_ptr<domain::ports::IVideoSource>> video_sources;
    std::vector<std::unique_ptr<domain::ports::IAngleSource>> angle_sources;
    std::unique_ptr<application::ports::IVideoAngleSourcesStorage> videoangle_sources_storage;
    std::unique_ptr<application::orchestrators::VideoSourceManager> video_source_manager;
    std::unique_ptr<domain::ports::IPressureSource> pressure_source;
};

SourcesAssembly assembleSources(
    const std::string& setup_dir,
    domain::ports::IClock& session_clock,
    domain::ports::IAngleCalculator& anglemeter,
    LoggerFactoryFn logger_factory,
    void* logger_context
);

std::unique_ptr<domain::ports::IAngleCalculator> assembleAnglemeter(const std::string& setup_dir, LoggerFactoryFn logger_factory, void* logger_context);
std::unique_ptr<domain::ports::ICalibrationCalculator> assembleCalibrator(const std::string& setup_dir, LoggerFactoryFn logger_factory, void* logger_context);

}

#endif //CLEANGRADUATOR_SOURCESASSEMBLER_H
