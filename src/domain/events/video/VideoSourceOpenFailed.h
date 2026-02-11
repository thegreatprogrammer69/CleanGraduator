#ifndef CLEANGRADUATOR_VIDEOSOURCEOPENFAILED_H
#define CLEANGRADUATOR_VIDEOSOURCEOPENFAILED_H
#include <string>

#include "domain/events/Event.h"

namespace domain::events {
    struct VideoSourceOpenFailed final : Event {
        std::string reason;
        EventType type() const override { return EventType::VideoSourceOpenFailed; }
        ~VideoSourceOpenFailed() override = default;
    };
}

#endif //CLEANGRADUATOR_VIDEOSOURCEOPENFAILED_H