#include "VideoSourceRepositoryFromFile.h"

#include <stdexcept>

#include "domain/ports/inbound/IVideoSource.h"
#include "infrastructure/video/linux/v4l/V4LCamera.h"
#include "infrastructure/video/linux/v4l/V4LCameraConfig.h"
#include "infrastructure/video/linux/gstreamer/GStreamerCamera.h"
#include "infrastructure/video/linux/gstreamer/GStreamerCameraConfig.h"
#include "infrastructure/utils/ini/IniFile.h"

namespace {
    utils::ini::IniFile loadIniOrThrow(const std::string& path) {
        utils::ini::IniFile ini;
        if (!ini.load(path)) {
            throw std::runtime_error("VideoSourceRepositoryFromFile: failed to load config: " + path);
        }
        return ini;
    }
}

infra::repo::VideoSourceRepositoryFromFile::VideoSourceRepositoryFromFile(const std::string& ini_path, camera::VideoSourcePorts video_source_ports)
{
    loadFromIniFile(ini_path, video_source_ports);
}

infra::repo::VideoSourceRepositoryFromFile::~VideoSourceRepositoryFromFile() {
}

std::vector<domain::ports::IVideoSource*> infra::repo::VideoSourceRepositoryFromFile::all() {
    return video_sources_ptr_;
}

void infra::repo::VideoSourceRepositoryFromFile::loadFromIniFile(const std::string& ini_path, camera::VideoSourcePorts video_source_ports) {
    auto ini = loadIniOrThrow(ini_path);

    for (int cam_idx = 0; cam_idx < 8; cam_idx++) {
        const auto section_name = "camera_" + std::to_string(cam_idx);
        auto section = ini[section_name];

        std::string backend = section.getString("backend", "");

#ifdef PLATFORM_LINUX
        if (backend == "v4l" || backend == "v4l2") {
            camera::V4LCameraConfig config{};
            config.source = section.getString("source", "");
            config.width = section.getInt("width", 0);
            config.height = section.getInt("height", 0);
            config.fps = section.getInt("fps", 0);
            video_sources_.push_back(std::make_unique<camera::V4LCamera>(video_source_ports, config));
        }
        else if (backend == "gst" || backend == "gstreamer") {
            camera::GStreamerCameraConfig config{};
            config.pipe = section.getString("pipe", "");
            video_sources_.push_back(std::make_unique<camera::GStreamerCamera>(video_source_ports, config));
        }

#elifdef PLATFORM_WINDOWS
        if (backend == "dshow") {
            // ...
        }
#endif
    }

}

