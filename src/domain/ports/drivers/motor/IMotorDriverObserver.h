#ifndef CLEANGRADUATOR_IMOTORDRIVEROBSERVER_H
#define CLEANGRADUATOR_IMOTORDRIVEROBSERVER_H
#include "domain/core/drivers/motor/MotorLimitsState.h"
#include "domain/core/drivers/motor/MotorDirection.h"

namespace domain::common {
    struct MotorError;
}

namespace domain::ports {
    struct IMotorDriverObserver {
        virtual ~IMotorDriverObserver() = default;

        virtual void onMotorStarted() = 0;
        virtual void onMotorStopped() = 0;
        virtual void onMotorStartFailed(const common::MotorError&) = 0;
        virtual void onMotorLimitsStateChanged(common::MotorLimitsState) = 0;
        virtual void onMotorDirectionChanged(common::MotorDirection) = 0;
    };
}

#endif //CLEANGRADUATOR_IMOTORDRIVEROBSERVER_H