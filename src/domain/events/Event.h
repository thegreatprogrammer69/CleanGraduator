#ifndef CLEANGRADUATOR_DOMAINEVENT_H
#define CLEANGRADUATOR_DOMAINEVENT_H
#include "EventCategory.h"

namespace domain::events {
    struct Event {
        virtual EventCategory category() const = 0;
        virtual ~Event() = default;
    };
}

#endif //CLEANGRADUATOR_DOMAINEVENT_H