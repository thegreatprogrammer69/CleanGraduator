#ifndef CLEANGRADUATOR_CALIBRATIONRESULTISSUETYPE_H
#define CLEANGRADUATOR_CALIBRATIONRESULTISSUETYPE_H

namespace domain::common {
    enum class CalibrationResultIssueType
    {
        AngleJump,
        NonMonotonicSequence,
        InconsistentSources,
        MissingCriticalCells,
        DirectionMismatch,
        SparseData
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONRESULTISSUETYPE_H