#ifndef CLEANGRADUATOR_VIDEOEVENTTYPE_H
#define CLEANGRADUATOR_VIDEOEVENTTYPE_H

namespace domain::events {
    enum class VideoEventType {
        VideoSourceClosed, VideoSourceOpened, VideoSourceOpenFailed, VideoSourceFrameCaptured
    };
}

#endif //CLEANGRADUATOR_VIDEOEVENTTYPE_H