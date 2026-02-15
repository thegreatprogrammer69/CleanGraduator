#ifndef CLEANGRADUATOR_IMOTORDRIVEROBSERVER_H
#define CLEANGRADUATOR_IMOTORDRIVEROBSERVER_H
#include "domain/core/motor/motor/MotorFault.h"

namespace domain::ports {
    struct IMotorDriverObserver {
        virtual ~IMotorDriverObserver() = default;

        virtual void onMotorStopped() = 0;
        virtual void onMotorEmergencyStop() = 0;
        virtual void onMotorFault(common::MotorFault fault) = 0;
    };
}

#endif //CLEANGRADUATOR_IMOTORDRIVEROBSERVER_H