#include "InfrastructureModule.h"

#include <stdexcept>
#include <string>

#include "infrastructure/camera/linux/gstreamer/GStreamerCamera.h"
#include "infrastructure/camera/linux/v4l/V4LCamera.h"
#include "infrastructure/camera/windows/dshow/DShowCamera.h"
#include "infrastructure/logging/ConsoleLogger.h"
#include "infrastructure/logging/FileLogger.h"
#include "infrastructure/process/ProcessRunner.h"
#include "infrastructure/settings/QtCameraGridSettingsRepository.h"
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

InfrastructureModule::CameraConfig InfrastructureModule::loadCameraConfig(
    const std::string& configDirectory,
    const std::string& fileName)
{
    const auto filePath = configDirectory + "/" + fileName;
    const auto ini = loadIniOrThrow(filePath);

    CameraConfig config;
    const auto& section = ini["camera"];
    config.implementation = section.getString("implementation", "");
    config.index = section.getInt("index", config.index);
    config.source = section.getString("source", config.source);
    config.pipeline = section.getString("pipeline", config.pipeline);
    config.width = section.getInt("width", config.width);
    config.height = section.getInt("height", config.height);
    config.fps = section.getInt("fps", config.fps);
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

std::unique_ptr<domain::ports::IVideoSource> InfrastructureModule::createCamera(
    const CameraConfig& config,
    const infra::camera::CameraPorts& ports)
{
#if defined(PLATFORM_WINDOWS)
    if (config.implementation == "dshow") {
        infra::camera::DShowCameraConfig cameraConfig{};
        cameraConfig.index = config.index;
        cameraConfig.width = config.width;
        cameraConfig.height = config.height;
        cameraConfig.fps = config.fps;
        return std::make_unique<infra::camera::DShowCamera>(ports, cameraConfig);
    }
#elif defined(PLATFORM_LINUX)
    if (config.implementation == "v4l") {
        infra::camera::V4LCameraConfig cameraConfig{};
        cameraConfig.source = config.source;
        cameraConfig.width = config.width;
        cameraConfig.height = config.height;
        cameraConfig.fps = config.fps;
        return std::make_unique<infra::camera::V4LCamera>(ports, cameraConfig);
    }

    if (config.implementation == "gstreamer") {
        infra::camera::GStreamerCameraConfig cameraConfig{};
        cameraConfig.pipe = config.pipeline;
        return std::make_unique<infra::camera::GStreamerCamera>(ports, cameraConfig);
    }
#endif

    throw std::runtime_error("Unsupported camera implementation for current platform: " + config.implementation);
}

std::unique_ptr<infrastructure::settings::QtCameraGridSettingsRepository> InfrastructureModule::createSettingsRepository(
    const std::string& configDirectory)
{
    const auto filePath = configDirectory + "/settings_repository.ini";
    const auto ini = loadIniOrThrow(filePath);
    const auto repositoryPath = ini["settings_repository"].getString("path", "settings_repo.ini");
    return std::make_unique<infrastructure::settings::QtCameraGridSettingsRepository>(repositoryPath);
}

std::unique_ptr<infra::process::ProcessRunner> InfrastructureModule::createProcessRunner(infra::process::ProcessLifecycle& lifecycle) {
    return std::make_unique<infra::process::ProcessRunner>(lifecycle);
}
