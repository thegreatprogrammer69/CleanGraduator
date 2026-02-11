#ifndef CLEANGRADUATOR_VIDEOSOURCEOPENED_H
#define CLEANGRADUATOR_VIDEOSOURCEOPENED_H
#include "domain/events/Event.h"

namespace domain::events {
    struct VideoSourceOpened final : VideoEvent {
        VideoEventType type() const override { return VideoEventType::VideoSourceOpened; }
    };
}
}

#endif //CLEANGRADUATOR_VIDEOSOURCEOPENED_H