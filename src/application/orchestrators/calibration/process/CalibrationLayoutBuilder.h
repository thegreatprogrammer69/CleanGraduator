#ifndef CLEANGRADUATOR_CALIBRATIONLAYOUTBUILDER_H
#define CLEANGRADUATOR_CALIBRATIONLAYOUTBUILDER_H

#include <set>

#include "CalibrationOrchestratorInput.h"
#include "domain/core/angle/SourceId.h"
#include "domain/ports/calibration/recording/ICalibrationRecorder.h"

namespace application::orchestrators {

class CalibrationLayoutBuilder final
{
public:
    domain::common::CalibrationLayout build(
        const CalibrationOrchestratorInput& input,
        const std::set<domain::common::SourceId>& openedAngleSources) const;
};

} // namespace application::orchestrators

#endif // CLEANGRADUATOR_CALIBRATIONLAYOUTBUILDER_H
