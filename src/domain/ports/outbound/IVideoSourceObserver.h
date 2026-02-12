#ifndef CLEANGRADUATOR_IFRAMESINK_H
#define CLEANGRADUATOR_IFRAMESINK_H

namespace domain::common {
    struct VideoSourceOpenError;
    struct VideoFramePacket;
}

namespace domain::ports {
    struct IVideoSourceObserver {
        virtual ~IVideoSourceObserver() = default;
        virtual void onVideoFrame(const common::VideoFramePacket&) = 0;
        virtual void onVideoSourceOpened() = 0;
        virtual void onVideoSourceOpenFailed(const common::VideoSourceOpenError&) = 0;
        virtual void onVideoSourceClosed() = 0;
    };
}
#endif //CLEANGRADUATOR_IFRAMESINK_H