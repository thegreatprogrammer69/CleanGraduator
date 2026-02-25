#ifndef CLEANGRADUATOR_IMOTORDRIVER_H
#define CLEANGRADUATOR_IMOTORDRIVER_H
#include <chrono>

#include "domain/core/drivers/motor/MotorFrequencyLimits.h"
#include "domain/core/drivers/motor/MotorLimitsState.h"
#include "domain/core/drivers/motor/MotorDirection.h"
#include "domain/core/drivers/motor/MotorDriverState.h"
#include "domain/core/drivers/motor/MotorDriverError.h"
#include "domain/core/drivers/motor/MotorFlapsState.h"
#include "domain/core/drivers/motor/MotorFrequency.h"

namespace domain::ports {
    struct IMotorDriverObserver;

    struct IMotorDriver {
        virtual ~IMotorDriver() = default;

        virtual bool initialize() = 0;

        // --- Lifecycle ---
        virtual bool start() = 0;
        virtual void stop() = 0;
        virtual common::MotorDriverState state() const = 0;

        // -- Safety API
        virtual void emergencyStop() = 0;
        virtual common::MotorDriverError error() const = 0;
        virtual void enableWatchdog(std::chrono::milliseconds timeout) = 0;
        virtual void disableWatchdog() = 0;

        // --- Motion config ---
        virtual void setFrequency(common::MotorFrequency) = 0;
        virtual common::MotorFrequency frequency() const = 0;
        virtual common::MotorFrequencyLimits frequencyLimits() const = 0;

        virtual void setDirection(common::MotorDirection) = 0;
        virtual common::MotorDirection direction() const = 0;

        // Клапана
        virtual void setFlapsState(common::MotorFlapsState) = 0;
        virtual common::MotorFlapsState flapsState() const = 0;

        // --- Limits ---
        virtual common::MotorLimitsState limits() const = 0;

        // --- Observers ---
        virtual void addObserver(IMotorDriverObserver&) = 0;
        virtual void removeObserver(IMotorDriverObserver&) = 0;
    };

}

#endif //CLEANGRADUATOR_IMOTORDRIVER_H