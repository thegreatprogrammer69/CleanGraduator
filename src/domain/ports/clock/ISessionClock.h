#ifndef CLEANGRADUATOR_ISESSIONCLOCK1_H
#define CLEANGRADUATOR_ISESSIONCLOCK1_H
#include "IClock.h"
#include "domain/core/measurement/Timestamp.h"

namespace domain::ports {
    struct ISessionClock : IClock {
        virtual ~ISessionClock() = default;
        virtual void start() = 0;
        virtual void stop() = 0;
    };
}

#endif //CLEANGRADUATOR_ISESSIONCLOCK1_H