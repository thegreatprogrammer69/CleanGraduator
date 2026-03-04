#ifndef CLEANGRADUATOR_CALIBRATIONCELLISSUE_H
#define CLEANGRADUATOR_CALIBRATIONCELLISSUE_H
#include <string>

#include "CalibrationIssueSeverity.h"
#include "domain/core/calibration/result/CalibrationCellKey.h"

namespace domain::common {
    struct CalibrationCellIssue
    {
        CalibrationCellKey cell;

        CalibrationIssueSeverity severity;

        enum class Type {
            MissingSamples,
            TooMuchNoise,
            NonMonotonicAngle,
            PressureOutOfRange
        };

        Type type;
        std::string message;
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONCELLISSUE_H