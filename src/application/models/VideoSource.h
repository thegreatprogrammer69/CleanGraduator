#ifndef CLEANGRADUATOR_VIDEOSOURCE_H
#define CLEANGRADUATOR_VIDEOSOURCE_H

namespace domain::ports {
    struct IVideoSource;
}

namespace application::models {
    struct VideoSource {
        domain::ports::IVideoSource& video_source;
        int id;
    };
}

#endif //CLEANGRADUATOR_VIDEOSOURCE_H