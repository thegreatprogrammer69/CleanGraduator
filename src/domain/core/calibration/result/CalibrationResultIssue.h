#ifndef CLEANGRADUATOR_CALIBRATIONRESULTISSUE_H
#define CLEANGRADUATOR_CALIBRATIONRESULTISSUE_H
#include <vector>
#include "CalibrationCellKey.h"

namespace domain::common {
    struct CalibrationResultIssue
    {
        std::vector<CalibrationCellKey> cells;
        CalibrationResultIssueType type;
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONRESULTISSUE_H