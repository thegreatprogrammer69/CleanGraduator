#ifndef CLEANGRADUATOR_IANGLESINK_H
#define CLEANGRADUATOR_IANGLESINK_H
#include "domain/core/angle/AngleSourceError.h"

namespace domain::common {
    struct AnglePacket;
}

namespace domain::ports {
    struct IAngleSourceObserver {
        virtual ~IAngleSourceObserver() = default;
        virtual void onStarted();
        virtual void onClosed();
        virtual void onError(const common::AngleSourceError&) = 0;
        virtual void onAngle(const common::AnglePacket&) = 0;
    };
}

#endif //CLEANGRADUATOR_IANGLESINK_H