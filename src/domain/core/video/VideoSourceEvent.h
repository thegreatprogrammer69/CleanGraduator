#ifndef CLEANGRADUATOR_VIDEOSOURCEEVENT_H
#define CLEANGRADUATOR_VIDEOSOURCEEVENT_H
#include <variant>
#include "VideoSourceError.h"

namespace domain::common {

    struct VideoSourceStarted {};
    struct VideoSourceStopped {};

    struct VideoSourceOpenFailed {
        VideoSourceError error;
    };

    using VideoSourceEvent = std::variant<
        VideoSourceStarted,
        VideoSourceStopped,
        VideoSourceOpenFailed
    >;

}

#endif //CLEANGRADUATOR_VIDEOSOURCEEVENT_H