#ifndef CLEANGRADUATOR_VIDEOSOURCECLOSED_H
#define CLEANGRADUATOR_VIDEOSOURCECLOSED_H
#include "domain/events/Event.h"

namespace domain::events {
    struct VideoSourceClosed final : Event {
        EventType type() const override { return EventType::VideoSourceClosed; }
    };
}

#endif //CLEANGRADUATOR_VIDEOSOURCECLOSED_H