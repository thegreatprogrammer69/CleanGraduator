#ifndef CLEANGRADUATOR_IMOTORDRIVER_H
#define CLEANGRADUATOR_IMOTORDRIVER_H
#include <chrono>

#include "domain/core/motor/motor/FrequencyLimits.h"
#include "domain/core/motor/motor/MotorLimitsState.h"
#include "domain/core/motor/motor/MotorDirection.h"
#include "domain/core/motor/motor/MotorFault.h"

namespace domain::ports {
    struct IMotorDriverObserver;

    struct IMotorDriver {
        virtual ~IMotorDriver() = default;

        // --- Lifecycle ---
        virtual void start() = 0;
        virtual void stop() = 0;
        virtual bool isRunning() const = 0;

        // Немедленный аварийный стоп (синхронный)
        virtual void emergencyStop() = 0;

        // --- Motion config ---
        virtual void setFrequency(int hz) = 0;
        virtual int frequency() const = 0;
        virtual common::FrequencyLimits frequencyLimits() const = 0;

        virtual void setDirection(common::MotorDirection dir) = 0;
        virtual common::MotorDirection direction() const = 0;

        // --- Limits ---
        virtual common::MotorLimitsState limits() const = 0;

        // --- Watchdog ---
        virtual void enableWatchdog(std::chrono::milliseconds timeout) = 0;
        virtual void disableWatchdog() = 0;
        virtual void feedWatchdog() = 0;

        // --- Fault state ---
        virtual common::MotorFault fault() const = 0;
        virtual void resetFault() = 0;

        // --- Observers ---
        virtual void addObserver(IMotorDriverObserver&) = 0;
        virtual void removeObserver(IMotorDriverObserver&) = 0;
    };

}

#endif //CLEANGRADUATOR_IMOTORDRIVER_H