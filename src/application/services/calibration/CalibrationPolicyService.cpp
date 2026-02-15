#include "CalibrationPolicyService.h"

#include <vector>

#include "domain/fmt/Logger.h"
#include "domain/ports/inbound/ICalibrationCalculator.h"

using namespace application::services;
using namespace domain::common;

CalibrationPolicyService::CalibrationPolicyService(domain::ports::ICalibrationCalculator &calculator, domain::ports::ILogger &logger)
    : calculator_(calculator), logger_(logger)
{
}

CalibrationResult CalibrationPolicyService::calculate(const CalibrationInput &input) const {
    fmt::Logger log(logger_);

    if (input.points.empty()) {
        CalibrationResult result;
        result.addMeasurement(0.0);
        result.markLast(CalibrationCellSeverity::Error, "calibration points are empty");
        log.error("calibration policy failed: no points provided");
        return result;
    }

    log.info("calibration policy started for {} target points", input.points.size());

    CalibrationResult result;
    for (const auto point : input.points) {
        CalibrationInput point_input {
            .points = {point},
            .angles = input.angles,
            .pressures = input.pressures,
        };

        auto point_result = calculator_.calculate(point_input);
        if (point_result.cells().empty()) {
            result.addMeasurement(0.0);
            result.markLast(CalibrationCellSeverity::Error, "calculator returned empty result");
            continue;
        }

        const auto& cell = point_result.cells().front();
        result.addMeasurement(cell.angle());
        result.markLast(cell.severity(), cell.message());
    }

    log.info("calibration policy completed with {} cells", result.cells().size());
    return result;
}

