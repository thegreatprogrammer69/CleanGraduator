#include "ClibrationResultValidation.h"

namespace domain::common {

void ClibrationResultValidation::clear() {
    issues_.clear();
}

void ClibrationResultValidation::addIssue(const CalibrationCellKey& key, ClibrationResultValidationIssue issue) {
    issues_[key].push_back(std::move(issue));
}

const ClibrationResultValidation::Issues* ClibrationResultValidation::issues(const CalibrationCellKey& key) const {
    const auto it = issues_.find(key);
    if (it == issues_.end()) {
        return nullptr;
    }
    return &it->second;
}

const std::unordered_map<CalibrationCellKey, ClibrationResultValidation::Issues>& ClibrationResultValidation::allIssues() const noexcept {
    return issues_;
}

bool ClibrationResultValidation::empty() const noexcept {
    return issues_.empty();
}

} // namespace domain::common
