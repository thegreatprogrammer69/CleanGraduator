#ifndef CLEANGRADUATOR_DOMAINEVENT_H
#define CLEANGRADUATOR_DOMAINEVENT_H
#include "EventType.h"

namespace domain::events {
    struct Event {
        virtual EventType type() const = 0;
        virtual ~Event() = default;
    };
}

#endif //CLEANGRADUATOR_DOMAINEVENT_H