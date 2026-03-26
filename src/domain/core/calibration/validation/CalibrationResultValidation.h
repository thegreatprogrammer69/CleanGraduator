#ifndef CLEANGRADUATOR_CALIBRATIONRESULTVALIDATION_H
#define CLEANGRADUATOR_CALIBRATIONRESULTVALIDATION_H

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "domain/core/calibration/analysis/CalibrationIssueSeverity.h"
#include "domain/core/calibration/result/CalibrationCellKey.h"

namespace domain::common {
    enum class CalibrationValidationIssueKind {
        AngleSpanTooHigh,
        AngleSpanTooLow,
        HysteresisExceeded,
    };

    struct CalibrationResultValidationIssue {
        CalibrationIssueSeverity severity;
        CalibrationValidationIssueKind kind;
        std::string message;

        bool operator==(const CalibrationResultValidationIssue& other) const {
            return severity == other.severity
                && kind == other.kind
                && message == other.message;
        }
    };

    class CalibrationResultValidation {
    public:
        using Issues = std::vector<CalibrationResultValidationIssue>;
        using Storage = std::unordered_map<CalibrationCellKey, Issues>;

        void setIssues(const CalibrationCellKey& key, Issues issues) {
            if (issues.empty()) {
                issues_.erase(key);
                return;
            }
            issues_[key] = std::move(issues);
        }

        void addIssue(const CalibrationCellKey& key, CalibrationResultValidationIssue issue) {
            issues_[key].push_back(std::move(issue));
        }

        const Issues& issuesFor(const CalibrationCellKey& key) const {
            static const Issues empty;
            const auto it = issues_.find(key);
            return it == issues_.end() ? empty : it->second;
        }

        const Storage& allIssues() const { return issues_; }
        bool empty() const noexcept { return issues_.empty(); }
        void clear() { issues_.clear(); }

        bool operator==(const CalibrationResultValidation& other) const {
            return issues_ == other.issues_;
        }

    private:
        Storage issues_;
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONRESULTVALIDATION_H
