#ifndef CLEANGRADUATOR_CALIBRATIONRESULTBUILDERPORTS_H
#define CLEANGRADUATOR_CALIBRATIONRESULTBUILDERPORTS_H
#include "domain/ports/calibration/recording/ICalibrationRecorder.h"
#include "domain/ports/logging/ILogger.h"

namespace domain::ports {
    class ICalibrationCalculator;
}

namespace application::orchestrators {
    struct CalibrationResultBuilderPorts {
        domain::ports::ILogger &logger;
        domain::ports::ICalibrationCalculator& calibration_calculator;
        domain::ports::ICalibrationRecorder& calibration_recorder;
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONRESULTBUILDERPORTS_H
