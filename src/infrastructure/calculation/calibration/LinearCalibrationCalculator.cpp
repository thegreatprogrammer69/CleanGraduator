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
    if (time.size() != values.size() || time.size() < 2) {
        throw std::invalid_argument("Invalid pressure series");
    }

    auto eval = [](float x0, float y0, float x1, float y1, float x) -> float {
        const float k = (x - x0) / (x1 - x0);
        return y0 + k * (y1 - y0);
    };

    bool has_segment = false;
    float best_result = 0.0f;
    float best_distance = 0.0f;

    for (size_t i = 0; i + 1 < values.size(); ++i) {
        const float v0 = values[i];
        const float v1 = values[i + 1];

        const float t0 = time[i];
        const float t1 = time[i + 1];

        // Пропускаем вырожденные сегменты
        if (v0 == v1 || t0 == t1) {
            continue;
        }

        // Обычная интерполяция: точка попала внутрь сегмента
        if ((value >= v0 && value <= v1) || (value >= v1 && value <= v0)) {
            return eval(v0, t0, v1, t1, value);
        }

        // Кандидат для экстраполяции: выбираем сегмент, ближайший к value
        const float dist = std::min(std::fabs(value - v0), std::fabs(value - v1));

        if (!has_segment || dist < best_distance) {
            has_segment = true;
            best_distance = dist;
            best_result = eval(v0, t0, v1, t1, value);
        }
    }

    if (!has_segment) {
        throw std::invalid_argument("No valid pressure segments for interpolation");
    }

    return best_result;
}

float interpolate_value(
    const std::vector<float>& time,
    const std::vector<float>& values,
    float t)
{
    if (time.size() != values.size() || time.size() < 2) {
        throw std::invalid_argument("Invalid angle series");
    }

    auto eval = [](float x0, float y0, float x1, float y1, float x) -> float {
        const float k = (x - x0) / (x1 - x0);
        return y0 + k * (y1 - y0);
    };

    bool has_segment = false;
    float best_result = 0.0f;
    float best_distance = 0.0f;

    for (size_t i = 0; i + 1 < time.size(); ++i) {
        const float t0 = time[i];
        const float t1 = time[i + 1];

        const float v0 = values[i];
        const float v1 = values[i + 1];

        // Пропускаем вырожденные сегменты
        if (t0 == t1 || v0 == v1) {
            continue;
        }

        // Обычная интерполяция
        if ((t >= t0 && t <= t1) || (t >= t1 && t <= t0)) {
            return eval(t0, v0, t1, v1, t);
        }

        // Кандидат для экстраполяции
        const float dist = std::min(std::fabs(t - t0), std::fabs(t - t1));

        if (!has_segment || dist < best_distance) {
            has_segment = true;
            best_distance = dist;
            best_result = eval(t0, v0, t1, v1, t);
        }
    }

    if (!has_segment) {
        throw std::invalid_argument("No valid angle segments for interpolation");
    }

    return best_result;
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