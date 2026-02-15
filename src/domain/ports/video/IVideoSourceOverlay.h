#ifndef CLEANGRADUATOR_IVIDEOSOURCEOVERLAY_H
#define CLEANGRADUATOR_IVIDEOSOURCEOVERLAY_H
#include "domain/core/common/VideoFramePacket.h"

namespace domain::ports {
    struct IVideoSourceOverlay {
        virtual domain::common::VideoFramePacket draw(common::VideoFramePacket) const = 0;
        virtual ~IVideoSourceOverlay() = default;
    };
}

#endif //CLEANGRADUATOR_IVIDEOSOURCEOVERLAY_H