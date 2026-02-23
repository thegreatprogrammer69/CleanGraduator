#ifndef CLEANGRADUATOR_IVIDEOSOURCEOVERLAY_H
#define CLEANGRADUATOR_IVIDEOSOURCEOVERLAY_H
#include "../../core/video/VideoFramePacket.h"

namespace domain::ports {
    struct IVideoSourceOverlay {
        virtual common::VideoFramePacket draw(common::VideoFramePacket) const = 0;
        virtual ~IVideoSourceOverlay() = default;
    };
}

#endif //CLEANGRADUATOR_IVIDEOSOURCEOVERLAY_H