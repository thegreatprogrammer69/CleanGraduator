#ifndef CLEANGRADUATOR_ICALIBRATIONRECORDEROBSERVER_H
#define CLEANGRADUATOR_ICALIBRATIONRECORDEROBSERVER_H
#include "domain/core/calibration/recording/CalibrationRecorderEvent.h"

namespace domain::ports {
    struct ICalibrationRecorderObserver {
        virtual ~ICalibrationRecorderObserver() = default;
        virtual void onCalibrationRecorderEvent(const common::CalibrationRecorderEvent& ev) = 0;
    };
}

#endif //CLEANGRADUATOR_ICALIBRATIONRECORDEROBSERVER_H