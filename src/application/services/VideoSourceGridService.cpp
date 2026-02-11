#include "VideoSourceGridService.h"

#include <sstream>

#include "application/dto/settings/grid/VideoSourceGridString.h"
#include <unordered_set>

#include "application/dto/settings/grid/VideoSourceGridSettings.h"
#include "application/ports/outbound/settings/IVideoSourceGridSettingsRepository.h"
#include "application/ports/outbound/video/IVideoSourceCrosshairListener.h"
#include "application/ports/outbound/video/IVideoSourceLifecycleObserver.h"
#include "domain/ports/inbound/IVideoSource.h"

using namespace application::services;

VideoSourceGridService::VideoSourceGridService(VideoSourceGridServicePorts ports, std::vector<VideoSource> video_sources)
    : video_sources_(std::move(video_sources))
    , settings_repo_(ports.settings_repo)
{
}

const std::vector<VideoSourceGridService::VideoSource> &VideoSourceGridService::sources() const {
    return video_sources_;
}

const std::vector<int> & VideoSourceGridService::indexes() const {
    return camera_grid_string_.indexes();
}

std::optional<VideoSourceGridService::VideoSource> VideoSourceGridService::sourceByIndex(int index) const {
    for (const auto& source : video_sources_) {
        if (source.index == index) {
            return source;
        }
    }
    return std::nullopt;
}

VideoSourceGridServiceApplyResult VideoSourceGridService::applyCameraSettings(const dto::VideoSourceGridSettings &settings) {
    const auto& active_indexes = settings.grid_string.indexes();
    const std::unordered_set<int> active_set(active_indexes.begin(),
                                       active_indexes.end());

    VideoSourceGridServiceApplyResult result;
    std::string correct_grid_string;
    std::stringstream errors;

    for (const auto& source : video_sources_) {
        source.crosshair_listener.onCrosshairChanged(settings.crosshair);
        if (active_set.count(source.index) > 0) {
            try {
                source.video_source.open();
                source.lifecycle_observer.onSourceOpened();
            }
            catch (const std::exception& e) {
                errors << "camera_" << source.index << " error:" << e.what() << std::endl;
                continue;
            }
            correct_grid_string += std::to_string(source.index);
        } else {
            source.video_source.close();
            source.lifecycle_observer.onSourceClosed();
        }
    }

    result.correct_settings = settings;
    result.correct_settings.grid_string = dto::VideoSourceGridString(correct_grid_string);
    result.error = errors.str();

    // Сохраняем настройки
    try {
        auto user_settings = settings_repo_.loadGridSettings();
        user_settings = result.correct_settings;
        settings_repo_.saveGridSettings(user_settings);
    }
    catch(const std::exception& e) {
        result.error = e.what();
    }

    return result;
}

VideoSourceGridServiceOpenResult VideoSourceGridService::openAllCameras() {
    VideoSourceGridServiceApplyResult result;

    std::string grid_string;
    for (const auto& source : video_sources_) {
        grid_string += std::to_string(source.index);
    }

    auto grid_settings = settings_repo_.loadGridSettings();
    grid_settings.grid_string = dto::VideoSourceGridString(grid_string);

    return applyCameraSettings(grid_settings);
}

