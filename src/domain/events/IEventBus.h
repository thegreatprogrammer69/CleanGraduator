#ifndef CLEANGRADUATOR_IEVENTBUS_H
#define CLEANGRADUATOR_IEVENTBUS_H

namespace domain::events {
    struct Event;
    struct IEventListener;

    class IEventBus {
    public:
        virtual ~IEventBus() = default;
        virtual void publish(const Event& event) = 0;
        virtual void addListener(IEventListener& listener) = 0;
        virtual void removeListener(IEventListener& listener) = 0;
    };
}

#endif //CLEANGRADUATOR_IEVENTBUS_H