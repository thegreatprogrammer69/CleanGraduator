#include "VideoSourceGridService.h"

#include <sstream>

#include "application/dto/settings/camera_grid/VideoSourceGridString.h"
#include <unordered_set>

#include "domain/ports/inbound/IVideoSource.h"

using namespace application::services;

VideoSourceGridService::VideoSourceGridService(std::vector<VideoSource> video_sources)
    : video_sources_(std::move(video_sources))
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

VideoSourceGridServiceApplyResult VideoSourceGridService::applyCameraString(
    const dto::VideoSourceGridString &camera_grid_string
)
{
    const auto& active_indexes = camera_grid_string.indexes();
    const std::unordered_set<int> active_set(active_indexes.begin(),
                                       active_indexes.end());

    std::string correct_camera_grid_string;
    std::stringstream errors;

    for (const auto& source : video_sources_) {
        if (active_set.count(source.index) > 0) {
            try {
                source.video_source.open();
            }
            catch (const std::exception& e) {
                errors << e.what() << std::endl;
                continue;
            }
            correct_camera_grid_string += std::to_string(source.index);
        } else {
            source.video_source.close();
        }
    }

    camera_grid_string_ = dto::VideoSourceGridString(correct_camera_grid_string);

    VideoSourceGridServiceApplyResult result {
        .correct_string = camera_grid_string_,
        .error = errors.str()
    };

    return result;
}
