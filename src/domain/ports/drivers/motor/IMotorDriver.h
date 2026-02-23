#ifndef CLEANGRADUATOR_IMOTORDRIVER_H
#define CLEANGRADUATOR_IMOTORDRIVER_H
#include <chrono>

#include "domain/core/drivers/motor/MotorFrequencyLimits.h"
#include "domain/core/drivers/motor/MotorLimitsState.h"
#include "domain/core/drivers/motor/MotorDirection.h"

namespace domain::ports {
    struct IMotorDriverObserver;

    struct IMotorDriver {
        virtual ~IMotorDriver() = default;

        virtual void initialize() = 0;

        // --- Lifecycle ---
        virtual bool start() = 0;
        virtual void stop() = 0;
        virtual bool isRunning() const = 0;

        // Немедленный аварийный стоп
        virtual void abort() = 0;

        // --- Motion config ---
        virtual void setFrequency(int hz) = 0;
        virtual int frequency() const = 0;
        virtual common::MotorFrequencyLimits frequencyLimits() const = 0;

        virtual void setDirection(common::MotorDirection dir) = 0;
        virtual common::MotorDirection direction() const = 0;

        // --- Limits ---
        virtual common::MotorLimitsState limits() const = 0;

        // --- Observers ---
        virtual void addObserver(IMotorDriverObserver&) = 0;
        virtual void removeObserver(IMotorDriverObserver&) = 0;
    };

}

#endif //CLEANGRADUATOR_IMOTORDRIVER_H