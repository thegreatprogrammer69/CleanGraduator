#ifndef CLEANGRADUATOR_ICALIBRATIONLIFECYCLEOBSERVER_H
#define CLEANGRADUATOR_ICALIBRATIONLIFECYCLEOBSERVER_H
#include <string>
#include "domain/core/calibration/lifecycle/CalibrationLifecycleState.h"

namespace domain::ports {
    struct ICalibrationLifecycleObserver {
        virtual ~ICalibrationLifecycleObserver() = default;
        virtual void onCalibrationLifecycleStateChanged(
            common::CalibrationLifecycleState newState,
            const std::string& lastError
        ) = 0;
    };
}

#endif //CLEANGRADUATOR_ICALIBRATIONLIFECYCLEOBSERVER_H