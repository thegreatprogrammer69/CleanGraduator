#ifndef CLEANGRADUATOR_VIDEOSOURCEFRAMECAPTURED_H
#define CLEANGRADUATOR_VIDEOSOURCEFRAMECAPTURED_H

#include "VideoEvent.h"
#include "domain/events/Event.h"

namespace domain::events {
    struct VideoSourceFrameCaptured final : VideoEvent {
        common::VideoFramePtr frame;
        VideoEventType type() const override { return VideoEventType::VideoSourceFrameCaptured; }
    };
}

#endif //CLEANGRADUATOR_VIDEOSOURCEFRAMECAPTURED_H