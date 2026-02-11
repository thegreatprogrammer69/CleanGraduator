#ifndef CLEANGRADUATOR_VIDEOSOURCEGRIDSERVICE_H
#define CLEANGRADUATOR_VIDEOSOURCEGRIDSERVICE_H
#include <optional>
#include <string>
#include <vector>

#include "VideoSourceGridServiceResult.h"

namespace application::ports {
    struct IVideoSourceGridSettingsRepository;
    struct IVideoSourceLifecycleObserver;
    struct IVideoSourceCrosshairListener;
}

namespace domain::ports {
    struct IVideoSource;
}

namespace application::dto {
    struct VideoSourceGridSettings;
    class VideoSourceGridString;
}

namespace application::services {

    struct VideoSourceGridServicePorts {
        ports::IVideoSourceGridSettingsRepository& settings_repo;
    };

    class VideoSourceGridService {
    public:
        struct VideoSource {
            int index;
            domain::ports::IVideoSource &video_source;
            ports::IVideoSourceCrosshairListener& crosshair_listener;
            ports::IVideoSourceLifecycleObserver& lifecycle_observer;
        };

        explicit VideoSourceGridService(VideoSourceGridServicePorts ports, std::vector<VideoSource> video_sources);

        const std::vector<VideoSource>& sources() const;
        const std::vector<int>& indexes() const;
        std::optional<VideoSource> sourceByIndex(int index) const;

        VideoSourceGridServiceApplyResult applyCameraSettings(const dto::VideoSourceGridSettings& settings);
        VideoSourceGridServiceOpenResult openAllCameras();

    private:
        std::vector<VideoSource> video_sources_;
        dto::VideoSourceGridString camera_grid_string_;
        ports::IVideoSourceGridSettingsRepository& settings_repo_;
    };
}


#endif //CLEANGRADUATOR_VIDEOSOURCEGRIDSERVICE_H