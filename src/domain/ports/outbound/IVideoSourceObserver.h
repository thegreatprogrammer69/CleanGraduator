#ifndef CLEANGRADUATOR_IFRAMESINK_H
#define CLEANGRADUATOR_IFRAMESINK_H

namespace domain::common {
    struct VideoSourceError;
    struct VideoFramePacket;
}

namespace domain::ports {
    struct IVideoSourceObserver {
        virtual ~IVideoSourceObserver() = default;
        virtual void onVideoFrame(const common::VideoFramePacket& packet) = 0;
        virtual void onVideoSourceOpened() = 0;
        virtual void onVideoSourceOpenFailed(const common::VideoSourceError&) = 0;
        virtual void onVideoSourceClosed(const common::VideoSourceError&) = 0;
    };
}
#endif //CLEANGRADUATOR_IFRAMESINK_H