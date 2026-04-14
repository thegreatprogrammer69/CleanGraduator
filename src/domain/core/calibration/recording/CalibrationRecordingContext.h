#ifndef CLEANGRADUATOR_CALIBRATIONRECORDINGCONTEXT_H
#define CLEANGRADUATOR_CALIBRATIONRECORDINGCONTEXT_H
#include "application/common/info/Gauge.h"
#include "domain/core/calibration/common/CalibrationLayout.h"
#include "domain/core/calibration/common/CalibrationMode.h"

namespace domain::common {
    struct CalibrationRecordingContext {
        CalibrationLayout layout;
        application::models::Gauge gauge;
        CalibrationMode calibration_mode{CalibrationMode::Full};
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONRECORDINGCONTEXT_H
