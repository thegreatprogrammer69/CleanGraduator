#ifndef CLEANGRADUATOR_VIDEOSOURCEOPENFAILED_H
#define CLEANGRADUATOR_VIDEOSOURCEOPENFAILED_H
#include <string>

#include "domain/events/Event.h"

namespace domain::events {
    struct VideoSourceOpenFiled final : VideoEvent {
        std::string reason;
        VideoEventType type() const override { return VideoEventType::VideoSourceOpenFailed; }
    };
}

#endif //CLEANGRADUATOR_VIDEOSOURCEOPENFAILED_H