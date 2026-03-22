#ifndef CLEANGRADUATOR_CLIBRATIONRESULTVALIDATION_H
#define CLEANGRADUATOR_CLIBRATIONRESULTVALIDATION_H

#include <string>
#include <unordered_map>
#include <vector>

#include "CalibrationCellKey.h"
#include "domain/core/calibration/analysis/CalibrationIssueSeverity.h"

namespace domain::common {

struct ClibrationResultValidationIssue {
    CalibrationIssueSeverity severity;
    std::string message;

    bool operator==(const ClibrationResultValidationIssue& other) const {
        return severity == other.severity && message == other.message;
    }
};

class ClibrationResultValidation {
public:
    using IssueMap = std::unordered_map<CalibrationCellKey, std::vector<ClibrationResultValidationIssue>>;

    void setIssues(const CalibrationCellKey& key, std::vector<ClibrationResultValidationIssue> issues);
    const std::vector<ClibrationResultValidationIssue>& issues(const CalibrationCellKey& key) const;
    const IssueMap& allIssues() const noexcept;
    bool empty() const noexcept;

    bool operator==(const ClibrationResultValidation& other) const;

private:
    IssueMap issues_by_cell_;
};

} // namespace domain::common

#endif // CLEANGRADUATOR_CLIBRATIONRESULTVALIDATION_H
