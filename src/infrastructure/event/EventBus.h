#ifndef CLEANGRADUATOR_EVENTBUS_H
#define CLEANGRADUATOR_EVENTBUS_H


#include <vector>
#include <mutex>
#include <algorithm>

#include "domain/events/IEventBus.h"

namespace infra::event {
    class EventBus : public domain::events::IEventBus {
    public:
        void publish(const domain::events::Event& event) override;

        void addListener(domain::events::IEventListener& handler) override;

        void removeListener(domain::events::IEventListener& handler) override;

    private:
        std::vector<domain::events::IEventListener*> listeners_;
        std::mutex mutex_;
    };
}



#endif //CLEANGRADUATOR_EVENTBUS_H