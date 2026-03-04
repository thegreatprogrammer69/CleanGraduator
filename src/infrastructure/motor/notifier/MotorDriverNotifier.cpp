#include "MotorDriverNotifier.h"
#include "domain/ports/drivers/motor/IMotorDriverObserver.h"
#include <algorithm>

namespace infra::motor {
    using domain::ports::IMotorDriverObserver;
    using domain::common::MotorDriverEvent;

    void MotorDriverNotifier::addObserver(IMotorDriverObserver& o) {
        observers_.add(o);
    }

    void MotorDriverNotifier::removeObserver(IMotorDriverObserver& o) {
        observers_.remove(o);
    }

    void MotorDriverNotifier::notifyEvent(const MotorDriverEvent& ev) {
        observers_.notify([&ev] (IMotorDriverObserver& o) {
           o.onMotorEvent(ev);
        });
    }
}
