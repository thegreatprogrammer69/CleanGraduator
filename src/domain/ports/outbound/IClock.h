#ifndef CLEANGRADUATOR_ISESSIONCLOCK_H
#define CLEANGRADUATOR_ISESSIONCLOCK_H
#include "domain/core/measurement/Timestamp.h"

namespace domain::ports {
    struct IClock {
        virtual ~IClock() = default;
        virtual common::Timestamp now() const = 0;
    };
}

#endif // CLEANGRADUATOR_ISESSIONCLOCK_H
