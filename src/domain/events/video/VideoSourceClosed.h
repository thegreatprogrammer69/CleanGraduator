#ifndef CLEANGRADUATOR_VIDEOSOURCECLOSED_H
#define CLEANGRADUATOR_VIDEOSOURCECLOSED_H
#include "VideoEvent.h"
#include "VideoEventType.h"
#include "domain/events/Event.h"

namespace domain::events {
    struct VideoSourceClosed final : VideoEvent {
        VideoEventType type() const override { return VideoEventType::VideoSourceClosed; }
    };
}

#endif //CLEANGRADUATOR_VIDEOSOURCECLOSED_H