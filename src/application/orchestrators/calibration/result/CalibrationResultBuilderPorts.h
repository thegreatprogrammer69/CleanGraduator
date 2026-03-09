#ifndef CLEANGRADUATOR_CALIBRATIONRESULTBUILDERPORTS_H
#define CLEANGRADUATOR_CALIBRATIONRESULTBUILDERPORTS_H
#include "domain/ports/calibration/recording/ICalibrationRecorder.h"
#include "domain/ports/logging/ILogger.h"

namespace application::orchestrators {
    struct CalibrationResultBuilderPorts {
        domain::ports::ILogger &logger;
        domain::ports::ICalibrationRecorder& calibration_recorder;
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONRESULTBUILDERPORTS_H