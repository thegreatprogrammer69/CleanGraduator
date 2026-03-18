#ifndef CLEANGRADUATOR_CALIBRATIONRECORDINGCONTEXT_H
#define CLEANGRADUATOR_CALIBRATIONRECORDINGCONTEXT_H
#include "application/models/info/Gauge.h"
#include "domain/core/calibration/common/CalibrationLayout.h"

namespace domain::common {
    struct CalibrationRecordingContext {
        CalibrationLayout layout;
        application::models::Gauge gauge;
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONRECORDINGCONTEXT_H