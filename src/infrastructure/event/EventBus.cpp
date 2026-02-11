#include "EventBus.h"

#include "domain/events/IEventListener.h"

void infra::event::EventBus::publish(const domain::events::Event &event) {
    std::vector<domain::events::IEventListener*> listenersCopy;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        listenersCopy = listeners_;  // копируем под мьютексом
    }

    // Вызываем вне мьютекса
    for (auto* listener : listenersCopy) {
        if (listener) {
            listener->onEvent(&event);
        }
    }
}

void infra::event::EventBus::addListener(domain::events::IEventListener &handler) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = std::find(listeners_.begin(), listeners_.end(), &handler);
    if (it == listeners_.end()) {
        listeners_.push_back(&handler);
    }
}

void infra::event::EventBus::removeListener(domain::events::IEventListener &handler) {
    std::lock_guard<std::mutex> lock(mutex_);

    listeners_.erase(
        std::remove(listeners_.begin(), listeners_.end(), &handler),
        listeners_.end()
    );
}
