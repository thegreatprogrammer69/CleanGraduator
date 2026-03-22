#ifndef CLEANGRADUATOR_CLIBRATIONRESULTVALIDATION_H
#define CLEANGRADUATOR_CLIBRATIONRESULTVALIDATION_H

#include <string>
#include <unordered_map>
#include <vector>

#include "CalibrationCellKey.h"

namespace domain::common {

struct ClibrationResultValidationIssue {
    enum class Type {
        ForwardAngleSpanOutOfRange,
        HysteresisExceeded,
    };

    Type type;
    std::string message;
};

class ClibrationResultValidation {
public:
    using Issues = std::vector<ClibrationResultValidationIssue>;

    void clear();
    void addIssue(const CalibrationCellKey& key, ClibrationResultValidationIssue issue);
    const Issues* issues(const CalibrationCellKey& key) const;
    const std::unordered_map<CalibrationCellKey, Issues>& allIssues() const noexcept;
    bool empty() const noexcept;

private:
    std::unordered_map<CalibrationCellKey, Issues> issues_;
};

} // namespace domain::common

namespace std {

template<>
struct hash<domain::common::CalibrationCellKey> {
    size_t operator()(const domain::common::CalibrationCellKey& key) const noexcept {
        size_t seed = std::hash<unsigned int>{}(key.point_id.id);
        seed ^= std::hash<int>{}(key.source_id.value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= std::hash<int>{}(static_cast<int>(key.direction)) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};

}

#endif // CLEANGRADUATOR_CLIBRATIONRESULTVALIDATION_H
