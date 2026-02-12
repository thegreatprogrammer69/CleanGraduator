#ifndef CLEANGRADUATOR_VIDEOSOURCE_H
#define CLEANGRADUATOR_VIDEOSOURCE_H

namespace domain::ports {
    struct IVideoSource;
}

namespace application::models {
    struct VideoSource {
        int id;
        domain::ports::IVideoSource& video_source;
    };
}

#endif //CLEANGRADUATOR_VIDEOSOURCE_H