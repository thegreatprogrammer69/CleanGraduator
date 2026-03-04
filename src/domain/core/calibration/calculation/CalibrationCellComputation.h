#ifndef CLEANGRADUATOR_CALIBRATIONCELLCOMPUTATION_H
#define CLEANGRADUATOR_CALIBRATIONCELLCOMPUTATION_H
#include <vector>
#include "domain/core/calibration/analysis/CalibrationCellIssue.h"
#include "domain/core/calibration/result/CalibrationCell.h"

namespace domain::common {

    struct CalibrationCellComputation
    {
        CalibrationCell cell;
        std::vector<CalibrationCellIssue> issues;
    };

}

#endif //CLEANGRADUATOR_CALIBRATIONCELLCOMPUTATION_H