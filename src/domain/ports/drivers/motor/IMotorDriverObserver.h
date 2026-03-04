#ifndef CLEANGRADUATOR_IMOTORDRIVEROBSERVER_H
#define CLEANGRADUATOR_IMOTORDRIVEROBSERVER_H

namespace domain::common {
    struct MotorDriverEvent;
}

namespace domain::ports {
    struct IMotorDriverObserver {
        virtual ~IMotorDriverObserver() = default;
        virtual void onMotorEvent(const common::MotorDriverEvent& event) = 0;
    };
}

#endif //CLEANGRADUATOR_IMOTORDRIVEROBSERVER_H