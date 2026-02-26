#ifndef CLEANGRADUATOR_VIDEOSOURCEEVENT_H
#define CLEANGRADUATOR_VIDEOSOURCEEVENT_H
#include <variant>
#include "VideoSourceError.h"

namespace domain::common {

    struct VideoSourceEvent {

        struct Opened {};
        struct OpenFailed { VideoSourceError error; };
        struct Failed { VideoSourceError error; };
        struct Closed {};

        using Data = std::variant<
            Opened,
            OpenFailed,
            Failed,
            Closed
        >;

        Data data;

        explicit VideoSourceEvent(Data data) : data(std::move(data)) {}
    };

}

#endif //CLEANGRADUATOR_VIDEOSOURCEEVENT_H