#ifndef CLEANGRADUATOR_IMOTORDRIVEROBSERVER_H
#define CLEANGRADUATOR_IMOTORDRIVEROBSERVER_H
#include "domain/core/motor/motor/MotorFault.h"
#include "domain/core/motor/motor/MotorLimitsState.h"

namespace domain::ports {
    struct IMotorDriverObserver {
        virtual ~IMotorDriverObserver() = default;

        virtual void onStarted() = 0;
        virtual void onStopped() = 0;
        virtual void onLimitsStateChanged(common::MotorLimitsState) = 0;
        virtual void onDirectionChanged(common::MotorDirection) = 0;
        virtual void onFault(const common::MotorFault&) = 0;
    };
}

#endif //CLEANGRADUATOR_IMOTORDRIVEROBSERVER_H