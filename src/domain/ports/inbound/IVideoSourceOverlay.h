#ifndef CLEANGRADUATOR_IVIDEOSOURCEOVERLAY_H
#define CLEANGRADUATOR_IVIDEOSOURCEOVERLAY_H
#include "domain/core/video/VideoFramePacket.h"

namespace domain::ports {
    struct IVideoSourceOverlay {
        virtual domain::common::VideoFramePacket draw(domain::common::VideoFramePacket) const = 0;
        virtual ~IVideoSourceOverlay() = default;
    };
}

#endif //CLEANGRADUATOR_IVIDEOSOURCEOVERLAY_H