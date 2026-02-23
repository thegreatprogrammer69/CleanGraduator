#ifndef CLEANGRADUATOR_ICALIBRATIONLIFECYCLEOBSERVER_H
#define CLEANGRADUATOR_ICALIBRATIONLIFECYCLEOBSERVER_H
#include "domain/core/calibration/lifecycle/CalibrationLifecycleState.h"

namespace domain::ports {
    struct ICalibrationLifecycleObserver {
        virtual void onCalibrationLifecycleStateChanged(common::CalibrationLifecycleState newState) = 0;
        virtual ~ICalibrationLifecycleObserver() = default;
    };
}

#endif //CLEANGRADUATOR_ICALIBRATIONLIFECYCLEOBSERVER_H