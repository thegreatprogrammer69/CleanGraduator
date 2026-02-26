#ifndef CLEANGRADUATOR_PRESSURESOURCENOTIFIER_H
#define CLEANGRADUATOR_PRESSURESOURCENOTIFIER_H

#include <algorithm>
#include <vector>
#include <mutex>

#include "infrastructure/utils/thread/ThreadSafeObserverList.h"

namespace domain::common {
    struct PressurePacket;
    struct PressureSourceEvent;
}

namespace domain::ports {
    struct IPressureSink;
    struct IPressureSourceObserver;
}

namespace infra::pressure::detail {

    class PressureSourceNotifier {
    public:
        void addObserver(domain::ports::IPressureSourceObserver& observer);
        void removeObserver(domain::ports::IPressureSourceObserver& observer);

        void addSink(domain::ports::IPressureSink& sink);
        void removeSink(domain::ports::IPressureSink& sink);

        void notifyPressure(const domain::common::PressurePacket& packet);
        void notifyEvent(const domain::common::PressureSourceEvent& event);

    private:
        ThreadSafeObserverList<domain::ports::IPressureSourceObserver> observers_;
        ThreadSafeObserverList<domain::ports::IPressureSink> sinks_;
    };

}

#endif // CLEANGRADUATOR_PRESSURESOURCENOTIFIER_H