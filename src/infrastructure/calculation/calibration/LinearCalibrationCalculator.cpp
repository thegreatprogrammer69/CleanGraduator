#include "LinearCalibrationCalculator.h"

#include <cmath>
#include <stdexcept>
#include <utility>
#include <vector>

using namespace infra::calc;
using namespace domain::common;

namespace {

    bool contains_error(const std::vector<CalibrationCellIssue>& issues)
    {
        for (const auto& issue : issues) {
            if (issue.severity == CalibrationIssueSeverity::Error) {
                return true;
            }
        }

        return false;
    }

    void validate_series_sizes(
        const CalibrationCalculatorInput& inp,
        std::vector<CalibrationCellIssue>& issues)
    {
        if (inp.pressure_series.time().size() != inp.pressure_series.value().size()) {
            issues.push_back(make_calibration_issue(
                CalibrationCellIssue::Type::PressureSeriesSizeMismatch,
                CalibrationIssueSeverity::Error));
        }

        if (inp.angle_series.time().size() != inp.angle_series.value().size()) {
            issues.push_back(make_calibration_issue(
                CalibrationCellIssue::Type::AngleSeriesSizeMismatch,
                CalibrationIssueSeverity::Error));
        }
    }

    void validate_pressure_series_count(
        const CalibrationCalculatorInput& inp,
        const LinearCalibrationCalculatorConfig& config,
        std::vector<CalibrationCellIssue>& issues)
    {
        const auto count = inp.pressure_series.time().size();

        if (count < static_cast<size_t>(config.min_pressure_values_in_series)) {
            issues.push_back(make_calibration_issue(
                CalibrationCellIssue::Type::TooFewPressureValues,
                CalibrationIssueSeverity::Error));
            return;
        }

        if (count < static_cast<size_t>(config.few_pressure_values_in_series)) {
            issues.push_back(make_calibration_issue(
                CalibrationCellIssue::Type::FewPressureValues,
                CalibrationIssueSeverity::Warning));
        }
    }

    void validate_angle_series_count(
        const CalibrationCalculatorInput& inp,
        const LinearCalibrationCalculatorConfig& config,
        std::vector<CalibrationCellIssue>& issues)
    {
        const auto count = inp.angle_series.time().size();

        if (count < static_cast<size_t>(config.min_angle_values_in_series)) {
            issues.push_back(make_calibration_issue(
                CalibrationCellIssue::Type::TooFewAngleValues,
                CalibrationIssueSeverity::Error));
            return;
        }

        if (count < static_cast<size_t>(config.few_angle_values_in_series)) {
            issues.push_back(make_calibration_issue(
                CalibrationCellIssue::Type::FewAngleValues,
                CalibrationIssueSeverity::Warning));
        }
    }

    void validate_input(
        const CalibrationCalculatorInput& inp,
        const LinearCalibrationCalculatorConfig& config,
        std::vector<CalibrationCellIssue>& issues)
    {
        validate_series_sizes(inp, issues);
        validate_pressure_series_count(inp, config, issues);
        validate_angle_series_count(inp, config, issues);
    }

    float interpolate_time(
        const std::vector<float>& time,
        const std::vector<float>& values,
        float value)
    {
        if (time.size() != values.size() || time.size() < 2)
            throw std::invalid_argument("Invalid pressure series");

        for (size_t i = 0; i + 1 < values.size(); ++i)
        {
            const float v0 = values[i];
            const float v1 = values[i + 1];

            const float t0 = time[i];
            const float t1 = time[i + 1];

            // пропускаем вырожденные сегменты
            if (v0 == v1 || t0 == t1)
                continue;

            if ((value >= v0 && value <= v1) || (value >= v1 && value <= v0))
            {
                const float k = (value - v0) / (v1 - v0);
                return t0 + k * (t1 - t0);
            }
        }

        throw std::out_of_range("Pressure point outside series range");
    }

    float interpolate_value(
        const std::vector<float>& time,
        const std::vector<float>& values,
        float t)
    {
        if (time.size() != values.size() || time.size() < 2)
            throw std::invalid_argument("Invalid angle series");

        for (size_t i = 0; i + 1 < time.size(); ++i)
        {
            const float t0 = time[i];
            const float t1 = time[i + 1];

            const float v0 = values[i];
            const float v1 = values[i + 1];

            // пропускаем вырожденные сегменты
            if (t0 == t1 || v0 == v1)
                continue;

            if ((t >= t0 && t <= t1) || (t >= t1 && t <= t0))
            {
                const float k = (t - t0) / (t1 - t0);
                return v0 + k * (v1 - v0);
            }
        }

        throw std::out_of_range("Time outside series range");
    }

} // namespace


LinearCalibrationCalculator::LinearCalibrationCalculator(
    CalibrationCalculatorPorts ports,
    LinearCalibrationCalculatorConfig config)
    : logger_(ports.logger)
    , config_(std::move(config))
{
}

LinearCalibrationCalculator::~LinearCalibrationCalculator() = default;


CalibrationCellComputation LinearCalibrationCalculator::compute(
    const CalibrationCalculatorInput& inp) const
{
    std::vector<CalibrationCellIssue> issues;
    std::optional<float> angle = std::nullopt;

    validate_input(inp, config_, issues);

    if (contains_error(issues)) {
        return { CalibrationCell(std::move(angle), std::move(issues)) };
    }

    try {
        const float time = interpolate_time(
            inp.pressure_series.time(),
            inp.pressure_series.value(),
            inp.pressure_point);

        angle = interpolate_value(
            inp.angle_series.time(),
            inp.angle_series.value(),
            time);
    }
    catch (const std::out_of_range&) {
        try {
            interpolate_time(
                inp.pressure_series.time(),
                inp.pressure_series.value(),
                inp.pressure_point);
        }
        catch (const std::out_of_range&) {
            issues.push_back(make_calibration_issue(
                CalibrationCellIssue::Type::PressurePointOutOfRange,
                CalibrationIssueSeverity::Error));

            return { CalibrationCell(std::move(angle), std::move(issues)) };
        }

        issues.push_back(make_calibration_issue(
            CalibrationCellIssue::Type::InterpolatedTimeOutOfRange,
            CalibrationIssueSeverity::Error));
    }
    catch (const std::invalid_argument&) {
        // Сюда не должны попадать после validate_input(),
        // но оставляем защиту на случай неконсистентных данных.
        if (inp.pressure_series.time().size() != inp.pressure_series.value().size()) {
            issues.push_back(make_calibration_issue(
                CalibrationCellIssue::Type::PressureSeriesSizeMismatch,
                CalibrationIssueSeverity::Error));
        }

        if (inp.angle_series.time().size() != inp.angle_series.value().size()) {
            issues.push_back(make_calibration_issue(
                CalibrationCellIssue::Type::AngleSeriesSizeMismatch,
                CalibrationIssueSeverity::Error));
        }
    }

    return { CalibrationCell(std::move(angle), std::move(issues)) };
}