#include "LinearResultCalculator.h"
#include <cmath>

using namespace infra::calculation;
using namespace domain::common;

namespace {
    struct calculateForPointResult {
        CalibrationCellSeverity severity;
        std::string message;
        double angle;
    };

    calculateForPointResult calculateForPoint(
        AngleSeries angle_series,
        PressureSeries pressure_series,
        const LinearResultCalculatorConfig& config
    )
{
    calculateForPointResult result;
    result.severity = CalibrationCellSeverity::None;
    result.message.clear();
    result.angle = 0.0;

    const auto& p_time  = pressure_series.time();
    const auto& p_value = pressure_series.value();

    const auto& a_time  = angle_series.time();
    const auto& a_value = angle_series.value();

    if (p_time.empty() || p_value.empty() ||
        a_time.empty() || a_value.empty())
    {
        result.severity = CalibrationCellSeverity::Error;
        result.message = "empty pressure or angle time series were obtained";
        return result;
    }

    if (p_time.size() != p_value.size() ||
        a_time.size() != a_value.size())
    {
        result.severity = CalibrationCellSeverity::Error;
        result.message = "time/value size mismatch in time series";
        return result;
    }

    using idx_t = std::size_t;

    idx_t pressure_from = p_time.size();
    idx_t pressure_to   = p_time.size();

    idx_t angle_from = a_time.size();
    idx_t angle_to   = a_time.size();

    // 1) Находим диапазон давлений [pressure_point - over, pressure_point + over]
    // pressure_point берём как центр диапазона
    const double pressure_point = 0.0; // <-- предполагаю, что ты задаёшь его извне раньше

    for (idx_t i = 0; i < p_value.size(); ++i) {
        if (p_value[i] > pressure_point - config.overlap) {
            pressure_from = i;
            break;
        }
    }

    if (pressure_from == p_time.size()) {
        result.severity = CalibrationCellSeverity::Error;
        result.message = "pressure_from not found";
        return result;
    }

    for (idx_t i = pressure_from; i < p_value.size(); ++i) {
        if (p_value[i] > pressure_point + config.overlap) {
            pressure_to = i;
            break;
        }
    }

    if (pressure_to == p_time.size()) {
        result.severity = CalibrationCellSeverity::Error;
        result.message = "pressure_to not found";
        return result;
    }

    // У вас было: нужно минимум 3 шага по давлению в диапазоне
    if (pressure_to - pressure_from < config.min_pressure_values_in_series) {
        result.severity = CalibrationCellSeverity::Error;
        result.message = "not enough pressure samples in range";
        return result;
    }

    const double time_from = p_time[pressure_from];
    const double time_to   = p_time[pressure_to];

    // 2) Находим диапазон углов по времени [time_from, time_to]
    for (idx_t i = 0; i < a_time.size(); ++i) {
        if (a_time[i] > time_from) {
            angle_from = i;
            break;
        }
    }

    if (angle_from == a_time.size()) {
        result.severity = CalibrationCellSeverity::Error;
        result.message = "angle_from not found";
        return result;
    }

    for (idx_t i = angle_from; i < a_time.size(); ++i) {
        if (a_time[i] > time_to) {
            angle_to = i;
            break;
        }
    }

    if (angle_to == a_time.size()) {
        result.severity = CalibrationCellSeverity::Error;
        result.message = "angle_to not found";
        return result;
    }

    // У вас было: нужно минимум 6 шагов по углу в диапазоне
    if (angle_to - angle_from < config.min_angle_values_in_series) {
        result.severity = CalibrationCellSeverity::Error;
        result.message = "not enough angle samples in range";
        return result;
    }

    // ------------------------------------------------------------
    // A) Интерполяция "время-давление": найти t*, когда P(t*) = pressure_point
    // ------------------------------------------------------------
    idx_t p1 = p_time.size();
    idx_t p2 = p_time.size();

    for (idx_t i = pressure_from; i + 1 < pressure_to; ++i) {
        const double P_a = p_value[i];
        const double P_b = p_value[i + 1];

        const bool crosses =
            (P_a <= pressure_point && pressure_point <= P_b) ||
            (P_b <= pressure_point && pressure_point <= P_a);

        if (crosses) {
            p1 = i;
            p2 = i + 1;
            break;
        }
    }

    if (p1 == p_time.size() || p2 == p_time.size()) {
        result.severity = CalibrationCellSeverity::Error;
        result.message = "pressure crossing not found";
        return result;
    }

    const double dP = p_value[p2] - p_value[p1];
    if (dP == 0.0) {
        result.severity = CalibrationCellSeverity::Error;
        result.message = "zero pressure delta";
        return result;
    }

    const double alpha =
        (pressure_point - p_value[p1]) / dP;

    const double t_star =
        p_time[p1] + alpha * (p_time[p2] - p_time[p1]);

    // ------------------------------------------------------------
    // B) Интерполяция "время-угол": найти angle(t_star)
    // ------------------------------------------------------------
    idx_t a1 = a_time.size();
    idx_t a2 = a_time.size();

    idx_t search_begin = (angle_from > 0) ? angle_from - 1 : angle_from;

    for (idx_t i = search_begin; i + 1 < angle_to; ++i) {
        const double T_a = a_time[i];
        const double T_b = a_time[i + 1];

        const bool crosses =
            (T_a <= t_star && t_star <= T_b) ||
            (T_b <= t_star && t_star <= T_a);

        if (crosses) {
            a1 = i;
            a2 = i + 1;
            break;
        }
    }

    if (a1 == a_time.size() || a2 == a_time.size()) {
        result.severity = CalibrationCellSeverity::Error;
        result.message = "angle crossing not found";
        return result;
    }

    const double dT = a_time[a2] - a_time[a1];
    if (dT == 0.0) {
        result.severity = CalibrationCellSeverity::Error;
        result.message = "zero time delta for angle";
        return result;
    }

    const double beta =
        (t_star - a_time[a1]) / dT;

    result.angle =
        a_value[a1] + beta * (a_value[a2] - a_value[a1]);

    result.severity = CalibrationCellSeverity::None;
    result.message = "ok";

    return result;
}

}

LinearResultCalculator::LinearResultCalculator(LinearResultCalculatorPorts ports, LinearResultCalculatorConfig config)
    : config_(config), logger_(ports.logger)
{
    logger_.info("constructor called");
}

CalibrationResult LinearResultCalculator::calculate(const CalibrationInput &input) const {
    logger_.info("start linear calibration for points {}, with {} angles and {} pressures", input.points, input.angles.size(), input.pressures.size());

    CalibrationResult result;
    for (int i = 0; i < input.pressures.size(); i++) {
        const auto [severity, message, angle] = calculateForPoint(input.angles, input.pressures, config_);
        result.addMeasurement(angle);
        result.markLast(severity, message);
        logger_.info("point {}: angle={}, severity={}, message='{}'", i, angle, severity, message);
    }

    logger_.info("calibration finished");
    return result;
}
