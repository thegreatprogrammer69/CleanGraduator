#include "InfrastructureModule.h"

#include <stdexcept>
#include <string>

#include "infrastructure/video/linux/gstreamer/GStreamerCamera.h"
#include "infrastructure/video/linux/v4l/V4LCamera.h"
#include "infrastructure/video/windows/dshow/DShowCamera.h"
#include "infrastructure/logging/ConsoleLogger.h"
#include "infrastructure/logging/FileLogger.h"
#include "infrastructure/process/ProcessRunner.h"
#include "infrastructure/settings/ISettingsRepository.h"
#include "infrastructure/utils/ini/IniFile.h"

using utils::ini::IniFile;

namespace {
IniFile loadIniOrThrow(const std::string& path) {
    IniFile ini;
    if (!ini.load(path)) {
        throw std::runtime_error("Failed to load config: " + path);
    }
    return ini;
}
} // namespace

InfrastructureModule::LoggerConfig InfrastructureModule::loadLoggerConfig(const std::string& configDirectory) {
    const auto filePath = configDirectory + "/logger.ini";
    const auto ini = loadIniOrThrow(filePath);

    LoggerConfig config;
    const auto& section = ini["logger"];
    config.implementation = section.getString("implementation", config.implementation);
    config.filePath = section.getString("file_path", config.filePath);
    return config;
}


std::unique_ptr<domain::ports::ILogger> InfrastructureModule::createLogger(const LoggerConfig& config) {
    if (config.implementation == "file") {
        return std::make_unique<infra::logging::FileLogger>(config.filePath);
    }

    if (config.implementation == "console") {
        return std::make_unique<infra::logging::ConsoleLogger>();
    }

    throw std::runtime_error("Unsupported logger implementation: " + config.implementation);
}

std::vector<std::unique_ptr<domain::ports::IVideoSource>> InfrastructureModule::createVideoSources(
    const std::string& configDirectory, const std::string& fileName,
    const infra::camera::VideoSourcePorts& ports)
{
    std::vector<std::unique_ptr<domain::ports::IVideoSource>> result;

    const auto filePath = configDirectory + "/" + fileName;
    const auto ini = loadIniOrThrow(filePath);

    for (int camIdx = 0; camIdx < 8; camIdx++) {
        const auto sectionName = "camera_" + std::to_string(camIdx);
        if (!ini.hasSection(sectionName)) {
            break;
        }
        const auto& section = ini[sectionName];
        std::string backend = section.getString("backend", "");

#if defined(PLATFORM_WINDOWS)
        if (backend == "dshow") {
            infra::camera::DShowCameraConfig cameraConfig{};
            cameraConfig.index = section.getInt("index", 0);
            cameraConfig.width = section.getInt("width", 0);
            cameraConfig.height = section.getInt("height", 0);
            cameraConfig.fps = section.getInt("fps", 0);
            result.push_back(std::make_unique<infra::camera::DShowCamera>(ports, cameraConfig));
            continue;
        }
#elif defined(PLATFORM_LINUX)
        if (backend == "v4l") {
            infra::camera::V4LCameraConfig cameraConfig{};
            cameraConfig.source = section.getString("source", "");
            cameraConfig.width = section.getInt("width", 0);
            cameraConfig.height = section.getInt("height", 0);
            cameraConfig.fps = section.getInt("fps", 0);
            result.push_back(std::make_unique<infra::camera::V4LCamera>(ports, cameraConfig));
            continue;
        }

        if (backend == "gstreamer") {
            infra::camera::GStreamerCameraConfig cameraConfig{};
            cameraConfig.pipe = section.getString("pipe", "");
            result.push_back(std::make_unique<infra::camera::GStreamerCamera>(ports, cameraConfig));
            continue;
        }
#endif

        throw std::runtime_error("unsupported camera implementation for current platform: " + backend);
    }
    return result;
}

std::unique_ptr<application::ports::IVideoSourceGridSettingsRepository> InfrastructureModule::createSettingsRepository(
    const std::string& configDirectory)
{
    const auto filePath = configDirectory + "/settings_repository.ini";
    const auto ini = loadIniOrThrow(filePath);
    const auto repositoryPath = ini["settings_repository"].getString("path", "settings_repo.ini");
    return std::make_unique<infrastructure::settings::IGridSettingsRepository>(repositoryPath);
}

std::unique_ptr<domain::ports::IProcessLifecycleObserver> InfrastructureModule::createProcessRunner(infra::process::ProcessLifecycle& lifecycle) {
    return std::make_unique<infra::process::ProcessRunner>(lifecycle);
}
