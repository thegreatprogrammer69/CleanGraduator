#ifndef CLEANGRADUATOR_EVENTTYPE_H
#define CLEANGRADUATOR_EVENTTYPE_H

namespace domain::events {
    enum class EventType {
        VideoSourceClosed, VideoSourceOpened, VideoSourceOpenFailed
    };
}

#endif //CLEANGRADUATOR_EVENTTYPE_H