#ifndef CLEANGRADUATOR_CALIBRATIONLAYOUTBUILDER_H
#define CLEANGRADUATOR_CALIBRATIONLAYOUTBUILDER_H

#include "CalibrationOrchestratorInput.h"
#include "domain/core/calibration/common/CalibrationLayout.h"
#include <set>

namespace application::orchestrators {

class CalibrationLayoutBuilder final {
public:
    domain::common::CalibrationLayout build(
        const CalibrationOrchestratorInput& input,
        const std::set<domain::common::SourceId>& opened_angle_sources) const;
};

} // namespace application::orchestrators

#endif // CLEANGRADUATOR_CALIBRATIONLAYOUTBUILDER_H
