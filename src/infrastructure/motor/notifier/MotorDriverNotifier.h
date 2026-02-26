#ifndef CLEANGRADUATOR_MOTORDRIVERNOTIFIER_H
#define CLEANGRADUATOR_MOTORDRIVERNOTIFIER_H

#include <vector>
#include <mutex>

#include "infrastructure/utils/thread/ThreadSafeObserverList.h"

namespace domain::common {
    struct MotorDriverEvent;
}

namespace domain::ports {
    struct IMotorDriverObserver;
}

namespace infra::motor {
    class MotorDriverNotifier {
    public:
        MotorDriverNotifier() = default;
        ~MotorDriverNotifier() = default;

        void addObserver(domain::ports::IMotorDriverObserver& o);
        void removeObserver(domain::ports::IMotorDriverObserver& o);

        void notifyEvent(const domain::common::MotorDriverEvent& ev);

    private:
        ThreadSafeObserverList<domain::ports::IMotorDriverObserver> observers_;


    };
}

#endif //CLEANGRADUATOR_MOTORDRIVERNOTIFIER_H