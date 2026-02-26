#ifndef CLEANGRADUATOR_VIDEOSOURCEEVENT_H
#define CLEANGRADUATOR_VIDEOSOURCEEVENT_H
#include <variant>
#include "VideoSourceError.h"

namespace domain::common {

    struct VideoSourceEvent {

        struct Opened {};
        struct OpenFailed { VideoSourceError error; };
        struct Closed {};

        using Data = std::variant<
            Opened,
            OpenFailed,
            Closed
        >;

        Data data;
    };

}

#endif //CLEANGRADUATOR_VIDEOSOURCEEVENT_H