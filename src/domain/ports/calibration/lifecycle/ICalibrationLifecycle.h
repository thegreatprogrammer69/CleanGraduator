#ifndef CLEANGRADUATOR_ICALIBRATIONLIFECYCLE_H
#define CLEANGRADUATOR_ICALIBRATIONLIFECYCLE_H
#include "domain/core/calibration/lifecycle/CalibrationLifecycleState.h"

namespace domain::ports {

    struct ICalibrationLifecycleObserver;

    struct ICalibrationLifecycle {
        virtual ~ICalibrationLifecycle() = default;

        virtual bool start() = 0;     // Stopped → Running
        virtual void stop() = 0;      // Running → Stopped

        virtual void fail(std::string error) = 0;

        virtual common::CalibrationLifecycleState state() const = 0;
        virtual std::string lastError() const = 0;

        virtual void addObserver(ICalibrationLifecycleObserver&) = 0;
        virtual void removeObserver(ICalibrationLifecycleObserver&) = 0;
    };

}

#endif //CLEANGRADUATOR_ICALIBRATIONLIFECYCLE_H