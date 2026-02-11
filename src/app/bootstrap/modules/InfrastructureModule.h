#ifndef CLEANGRADUATOR_INFRASTRUCTUREMODULE_H
#define CLEANGRADUATOR_INFRASTRUCTUREMODULE_H

#include <memory>
#include <string>

#include "domain/ports/outbound/ILogger.h"
#include "domain/ports/inbound/IVideoSource.h"
#include "infrastructure/video/VideoSourcePorts.h"
#include "infrastructure/process/ProcessLifecycle.h"

namespace application::ports {
    struct IVideoSourceGridSettingsRepository;
}

namespace infrastructure::settings {
class QtCameraGridSettingsRepository;
}

namespace infra::process {
class ProcessRunner;
}

struct InfrastructureModule {
    struct LoggerConfig {
        std::string implementation{"console"};
        std::string filePath{"app.log"};
    };


    static LoggerConfig loadLoggerConfig(const std::string& configDirectory);

    static std::unique_ptr<domain::ports::ILogger> createLogger(const LoggerConfig& config);

    static std::vector<std::unique_ptr<domain::ports::IVideoSource>> createVideoSources(
        const std::string &configDirectory, const std::string &fileName, const infra::camera::VideoSourcePorts &ports);


    static std::unique_ptr<application::ports::IVideoSourceGridSettingsRepository> createSettingsRepository(
        const std::string& configDirectory);

    static std::unique_ptr<domain::ports::IProcessLifecycleObserver> createProcessRunner(infra::process::ProcessLifecycle& lifecycle);
};

#endif //CLEANGRADUATOR_INFRASTRUCTUREMODULE_H
