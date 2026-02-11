#include "VideoSourceFactory.h"

#include <stdexcept>

#include "domain/ports/inbound/IVideoSource.h"
#include "infrastructure/video/linux/v4l/V4LCamera.h"
#include "infrastructure/video/linux/v4l/V4LCameraConfig.h"
#include "infrastructure/video/linux/gstreamer/GStreamerCamera.h"
#include "infrastructure/video/linux/gstreamer/GStreamerCameraConfig.h"
#include "infrastructure/video/windows/dshow/DShowCameraConfig.h"
#include "infrastructure/video/windows/dshow/DShowCamera.h"
#include "infrastructure/utils/ini/IniFile.h"

namespace {
    utils::ini::IniFile loadIniOrThrow(const std::string& path) {
        utils::ini::IniFile ini;
        if (!ini.load(path)) {
            throw std::runtime_error("VideoSourceFactory: failed to load config: " + path);
        }
        return ini;
    }
}

infra::repo::VideoSourceFactory::VideoSourceFactory(const std::string& ini_path, camera::VideoSourcePorts ports)
    : ini_path_(ini_path), ports_(ports)
{

}

infra::repo::VideoSourceFactory::~VideoSourceFactory() {
}


std::vector<std::unique_ptr<domain::ports::IVideoSource>> infra::repo::VideoSourceFactory::load() {
    auto ini = loadIniOrThrow(ini_path_);

    std::vector<std::unique_ptr<domain::ports::IVideoSource>> result;

    for (int cam_idx = 0; cam_idx < 8; cam_idx++) {
        const auto section_name = "camera_" + std::to_string(cam_idx);
        auto section = ini[section_name];

        if (!ini.hasSection(section_name)) break;

        std::string backend = section.getString("backend", "");

#ifdef PLATFORM_LINUX
        if (backend == "v4l" || backend == "v4l2") {
            camera::V4LCameraConfig config{};
            config.source = section.getString("source", "");
            config.width = section.getInt("width", 480);
            config.height = section.getInt("height", 640);
            config.fps = section.getInt("fps", 30);
            result.push_back(std::make_unique<camera::V4LCamera>(ports_, config));
            continue;
        }
        else if (backend == "gst" || backend == "gstreamer") {
            camera::GStreamerCameraConfig config{};
            config.pipe = section.getString("pipe", "");
            result.push_back(std::make_unique<camera::GStreamerCamera>(ports_, config));
            continue;
        }

#elifdef PLATFORM_WINDOWS
        if (backend == "dshow") {
            camera::DShowCameraConfig config{};
            config.index = section.getString("index", "");
            config.width = section.getInt("width", 480);
            config.height = section.getInt("height", 640);
            config.fps = section.getInt("fps", 30);
            result.push_back(std::make_unique<camera::DShowCamera>(ports_, config));
            continue;
;        }
#endif
        throw std::runtime_error("unsupported camera implementation for current platform: " + backend);
    }

    return result;
}

