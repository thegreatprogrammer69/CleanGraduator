#ifndef CLEANGRADUATOR_IFRAMESINK_H
#define CLEANGRADUATOR_IFRAMESINK_H
#include <chrono>
#include "domain/core/video/VideoFrame.h"

namespace domain::common {
    class Timestamp;
}

namespace domain::ports {
    struct IVideoSink {
        virtual ~IVideoSink() = default;
        virtual void onVideoFrame(const common::Timestamp&, common::VideoFramePtr) = 0;
    };
}
#endif //CLEANGRADUATOR_IFRAMESINK_H