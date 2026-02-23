#ifndef CLEANGRADUATOR_IANGLESINK_H
#define CLEANGRADUATOR_IANGLESINK_H
#include "domain/core/angle/AngleSourceError.h"

namespace domain::common {
    struct AngleSourcePacket;
}

namespace domain::ports {

    struct IAngleSourceObserver {
        virtual ~IAngleSourceObserver() = default;

        virtual void onAngleSourceStarted() = 0;
        virtual void onAngleSourceStopped() = 0;
        virtual void onAngleSourceFailed(const common::AngleSourceError&) = 0;

        virtual void onAngleSourcePacket(const common::AngleSourcePacket&) = 0;
    };

}

#endif //CLEANGRADUATOR_IANGLESINK_H