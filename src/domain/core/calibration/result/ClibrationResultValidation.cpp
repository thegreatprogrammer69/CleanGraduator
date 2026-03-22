#include "ClibrationResultValidation.h"

namespace domain::common {

void ClibrationResultValidation::setIssues(
    const CalibrationCellKey& key,
    std::vector<ClibrationResultValidationIssue> issues)
{
    if (issues.empty()) {
        issues_by_cell_.erase(key);
        return;
    }

    auto& bucket = issues_by_cell_[key];
    bucket.insert(bucket.end(), issues.begin(), issues.end());
}

const std::vector<ClibrationResultValidationIssue>&
ClibrationResultValidation::issues(const CalibrationCellKey& key) const
{
    static const std::vector<ClibrationResultValidationIssue> empty_issues;

    const auto it = issues_by_cell_.find(key);
    if (it == issues_by_cell_.end()) {
        return empty_issues;
    }

    return it->second;
}

const ClibrationResultValidation::IssueMap& ClibrationResultValidation::allIssues() const noexcept
{
    return issues_by_cell_;
}

bool ClibrationResultValidation::empty() const noexcept
{
    return issues_by_cell_.empty();
}

bool ClibrationResultValidation::operator==(const ClibrationResultValidation& other) const
{
    return issues_by_cell_ == other.issues_by_cell_;
}

} // namespace domain::common
