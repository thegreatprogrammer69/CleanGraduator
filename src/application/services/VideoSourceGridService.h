#ifndef CLEANGRADUATOR_VIDEOSOURCEGRIDSERVICE_H
#define CLEANGRADUATOR_VIDEOSOURCEGRIDSERVICE_H
#include <optional>
#include <string>
#include <vector>

#include "VideoSourceGridServiceApplyResult.h"
#include "application/dto/settings/camera_grid/VideoSourceGridString.h"

namespace domain::ports {
    struct IVideoSource;
}

namespace application::dto {
    class VideoSourceGridString;
}

namespace application::services {

    class VideoSourceGridService {
    public:
        struct VideoSource {
            int index;
            domain::ports::IVideoSource &video_source;
        };

        explicit VideoSourceGridService(std::vector<VideoSource> video_sources);

        const std::vector<VideoSource>& sources() const;
        const std::vector<int>& indexes() const;
        std::optional<VideoSource> sourceByIndex(int index) const;

        VideoSourceGridServiceApplyResult applyCameraString(const dto::VideoSourceGridString&);

    private:
        std::vector<VideoSource> video_sources_;
        dto::VideoSourceGridString camera_grid_string_;
    };
}


#endif //CLEANGRADUATOR_VIDEOSOURCEGRIDSERVICE_H