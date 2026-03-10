#ifndef CLEANGRADUATOR_CALIBRATIONCELL_H
#define CLEANGRADUATOR_CALIBRATIONCELL_H

#include <optional>
#include "domain/core/calibration/analysis/CalibrationCellIssue.h"


namespace domain::common {

    class CalibrationCell {
    public:
        CalibrationCell() = default;
        CalibrationCell(std::optional<float> angle, std::vector<CalibrationCellIssue> issues)
            : angle_(std::move(angle)), issues_(std::move(issues))
        {}

        std::optional<float> angle() const { return angle_; }
        const std::vector<CalibrationCellIssue>& issues() const { return issues_; }

        bool operator==(const CalibrationCell& other) const {
            return angle_ == other.angle_;
        }

    private:
        std::optional<float> angle_{};
        std::vector<CalibrationCellIssue> issues_{};
    };

}

#endif //CLEANGRADUATOR_CALIBRATIONCELL_H