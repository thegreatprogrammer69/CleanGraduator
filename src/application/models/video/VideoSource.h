#ifndef CLEANGRADUATOR_VIDEOSOURCE_H
#define CLEANGRADUATOR_VIDEOSOURCE_H

#include "domain/core/angle/SourceId.h"

namespace domain::ports {
    struct IAngleSource;
    struct IVideoSource;
}

namespace application::models {
    struct VideoAngleSource {
        domain::common::SourceId id;
        domain::ports::IAngleSource& angle_source;
        domain::ports::IVideoSource& video_source;
    };
}

#endif //CLEANGRADUATOR_VIDEOSOURCE_H