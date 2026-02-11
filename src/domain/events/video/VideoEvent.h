#ifndef CLEANGRADUATOR_VIDEOEVENT_H
#define CLEANGRADUATOR_VIDEOEVENT_H
#include "VideoEventType.h"
#include "domain/events/Event.h"

namespace domain::events {
    struct VideoEvent : Event {
        EventCategory category() const override { return EventCategory::Video; }
        virtual VideoEventType type() const = 0;
        virtual ~VideoEvent() = default;
    };
}

#endif //CLEANGRADUATOR_VIDEOEVENT_H