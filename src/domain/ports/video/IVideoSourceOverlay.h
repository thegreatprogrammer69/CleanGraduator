#ifndef CLEANGRADUATOR_IVIDEOSOURCEOVERLAY_H
#define CLEANGRADUATOR_IVIDEOSOURCEOVERLAY_H

namespace domain::common {
    struct VideoFramePacket;
}

namespace domain::ports {
    struct IVideoSourceOverlay {
        virtual void draw(const common::VideoFramePacket&) const = 0;
        virtual ~IVideoSourceOverlay() = default;
    };
}

#endif //CLEANGRADUATOR_IVIDEOSOURCEOVERLAY_H