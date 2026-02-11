#ifndef CLEANGRADUATOR_INFRASTRUCTUREMODULE_H
#define CLEANGRADUATOR_INFRASTRUCTUREMODULE_H

#include <memory>
#include <string>

#include "domain/ports/outbound/ILogger.h"
#include "domain/ports/inbound/IVideoSource.h"
#include "infrastructure/camera/CameraPorts.h"
#include "infrastructure/process/ProcessLifecycle.h"

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

    struct CameraConfig {
        std::string implementation;
        int index{0};
        std::string source{"/dev/video0"};
        std::string pipeline;
        int width{640};
        int height{480};
        int fps{30};
    };

    static LoggerConfig loadLoggerConfig(const std::string& configDirectory);
    static CameraConfig loadCameraConfig(const std::string& configDirectory, const std::string& fileName);

    static std::unique_ptr<domain::ports::ILogger> createLogger(const LoggerConfig& config);
    static std::unique_ptr<domain::ports::IVideoSource> createCamera(
        const CameraConfig& config,
        const infra::camera::CameraPorts& ports);
    static std::unique_ptr<infrastructure::settings::QtCameraGridSettingsRepository> createSettingsRepository(
        const std::string& configDirectory);
    static std::unique_ptr<infra::process::ProcessRunner> createProcessRunner(infra::process::ProcessLifecycle& lifecycle);
};

#endif //CLEANGRADUATOR_INFRASTRUCTUREMODULE_H
