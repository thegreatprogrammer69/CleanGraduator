#ifndef CLEANGRADUATOR_IEVENTLISTENER_H
#define CLEANGRADUATOR_IEVENTLISTENER_H

namespace domain::events {
    struct Event;

    class IEventListener {
    public:
        virtual ~IEventListener() = default;
        virtual void onEvent(const Event* event) = 0;
    };
}

#endif //CLEANGRADUATOR_IEVENTLISTENER_H