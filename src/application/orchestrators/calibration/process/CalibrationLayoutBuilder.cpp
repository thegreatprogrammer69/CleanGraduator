#include "CalibrationLayoutBuilder.h"

#include <vector>

using namespace application::orchestrators;
using namespace domain::common;

CalibrationLayout CalibrationLayoutBuilder::build(
    const CalibrationOrchestratorInput& input,
    const std::set<SourceId>& openedAngleSources) const
{
    CalibrationLayout layout;

    layout.sources = std::vector<SourceId>(
        openedAngleSources.begin(),
        openedAngleSources.end());

    layout.directions.push_back(MotorDirection::Forward);

    if (input.calibration_mode == CalibrationMode::Full)
    {
        layout.directions.push_back(MotorDirection::Backward);
    }

    int i = 0;
    for (const auto& point : input.gauge.points.value)
    {
        layout.points.push_back(PointId(i, point));
        ++i;
    }

    return layout;
}
