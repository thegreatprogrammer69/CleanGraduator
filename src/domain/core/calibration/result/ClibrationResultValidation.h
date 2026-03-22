#ifndef CLEANGRADUATOR_CLIBRATIONRESULTVALIDATION_H
#define CLEANGRADUATOR_CLIBRATIONRESULTVALIDATION_H

#include <string>
#include <unordered_map>
#include <vector>

#include "domain/core/calibration/analysis/CalibrationIssueSeverity.h"
#include "CalibrationCellKey.h"

namespace domain::common {

struct ClibrationResultValidationIssue {
    CalibrationIssueSeverity severity;
    std::string message;

    bool operator==(const ClibrationResultValidationIssue& rhs) const {
        return severity == rhs.severity && message == rhs.message;
    }
};

class ClibrationResultValidation {
public:
    using Issues = std::vector<ClibrationResultValidationIssue>;
    using Storage = std::unordered_map<CalibrationCellKey, Issues>;

    void setIssues(const CalibrationCellKey& key, Issues issues) {
        if (issues.empty()) {
            issues_.erase(key);
            return;
        }
        issues_[key] = std::move(issues);
    }

    const Issues* issuesFor(const CalibrationCellKey& key) const {
        const auto it = issues_.find(key);
        return it == issues_.end() ? nullptr : &it->second;
    }

    const Storage& items() const noexcept { return issues_; }
    bool empty() const noexcept { return issues_.empty(); }
    void clear() noexcept { issues_.clear(); }

    bool operator==(const ClibrationResultValidation& rhs) const { return issues_ == rhs.issues_; }

private:
    Storage issues_;
};

} // namespace domain::common

#endif // CLEANGRADUATOR_CLIBRATIONRESULTVALIDATION_H
