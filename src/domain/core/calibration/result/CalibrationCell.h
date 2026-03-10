#ifndef CLEANGRADUATOR_CALIBRATIONCELL_H
#define CLEANGRADUATOR_CALIBRATIONCELL_H

#include <optional>
#include "domain/core/calibration/analysis/CalibrationCellIssue.h"


namespace domain::common {

    class CalibrationCell {
    public:
        CalibrationCell() = default;
        CalibrationCell(const float angle, std::vector<CalibrationCellIssue> issues)
            : angle_(angle), issues_(std::move(issues))
        {}

        float angle() const { return angle_; }
        const std::vector<CalibrationCellIssue>& issues() { return issues_; }

    private:
        float angle_{};
        std::vector<CalibrationCellIssue> issues_{};
    };

}

#endif //CLEANGRADUATOR_CALIBRATIONCELL_H