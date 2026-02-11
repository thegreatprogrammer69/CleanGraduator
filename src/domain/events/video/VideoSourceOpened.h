#ifndef CLEANGRADUATOR_VIDEOSOURCEOPENED_H
#define CLEANGRADUATOR_VIDEOSOURCEOPENED_H
#include "domain/events/Event.h"

namespace domain::events {
    struct VideoSourceOpened final : Event {
        EventType type() const override { return EventType::VideoSourceOpened; }
    };
}

#endif //CLEANGRADUATOR_VIDEOSOURCEOPENED_H