#include "MotorDriverNotifier.h"
#include "domain/ports/drivers/motor/IMotorDriverObserver.h"
#include <algorithm>

namespace infra::motor {
    using domain::ports::IMotorDriverObserver;
    using domain::common::MotorDriverEvent;

    void MotorDriverNotifier::addObserver(IMotorDriverObserver& observer)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        auto it = std::find(observers_.begin(), observers_.end(), &observer);
        if (it == observers_.end()) {
            observers_.push_back(&observer);
        }
    }

    void MotorDriverNotifier::removeObserver(IMotorDriverObserver& observer)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        observers_.erase(
            std::remove(observers_.begin(), observers_.end(), &observer),
            observers_.end()
        );
    }

    void MotorDriverNotifier::notify(const MotorDriverEvent& event)
    {
        std::vector<IMotorDriverObserver*> snapshot;

        {
            // Копируем список под mutex
            std::lock_guard<std::mutex> lock(mutex_);
            snapshot = observers_;
        }

        // Вызываем callbacks БЕЗ mutex
        for (auto* observer : snapshot) {
            if (observer) {
                observer->onMotorEvent(event);
            }
        }
    }
}
