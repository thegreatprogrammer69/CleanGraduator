#ifndef CLEANGRADUATOR_ICALIBRATIONMOTIONOBSERVER_H
#define CLEANGRADUATOR_ICALIBRATIONMOTIONOBSERVER_H
#include "MotionAbortReason.h"
#include "domain/core/calibration/calculation/CalibrationMotionPhase.h"

namespace domain::ports {

    struct ICalibrationMotionObserver {
        virtual ~ICalibrationMotionObserver() = default;

        virtual void onPhaseChanged(common::CalibrationMotionPhase phase) = 0;
        virtual void onCompleted() = 0;
        virtual void onAborted(MotionAbortReason reason) = 0;
    };

}

#endif //CLEANGRADUATOR_ICALIBRATIONMOTIONOBSERVER_H