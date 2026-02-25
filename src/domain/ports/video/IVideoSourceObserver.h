#ifndef CLEANGRADUATOR_IFRAMESINK_H
#define CLEANGRADUATOR_IFRAMESINK_H
#include "domain/core/video/VideoSourceEvent.h"

namespace domain::ports {
    struct IVideoSourceObserver {
        virtual ~IVideoSourceObserver() = default;
        virtual void onVideoSourceEvent(const common::VideoSourceEvent&) = 0;
    };
}
#endif //CLEANGRADUATOR_IFRAMESINK_H