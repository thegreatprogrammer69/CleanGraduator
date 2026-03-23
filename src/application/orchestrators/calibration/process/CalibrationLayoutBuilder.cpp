#include "CalibrationLayoutBuilder.h"

#include <stdexcept>

using namespace application::orchestrators;
using namespace domain::common;

CalibrationLayout CalibrationLayoutBuilder::build(
    const CalibrationOrchestratorInput& input,
    const std::set<SourceId>& opened_angle_sources) const
{
    if (opened_angle_sources.empty()) {
        throw std::invalid_argument("CalibrationLayoutBuilder: no opened angle sources");
    }

    CalibrationLayout layout;
    layout.sources.assign(opened_angle_sources.begin(), opened_angle_sources.end());
    layout.directions.push_back(MotorDirection::Forward);

    if (input.calibration_mode == CalibrationMode::Full) {
        layout.directions.push_back(MotorDirection::Backward);
    }

    unsigned int point_index = 0;
    for (const auto& pressure_point : input.gauge.points.value) {
        layout.points.emplace_back(point_index, pressure_point);
        ++point_index;
    }

    return layout;
}
