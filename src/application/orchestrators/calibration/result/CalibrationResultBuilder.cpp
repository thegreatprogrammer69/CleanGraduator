#include "CalibrationResultBuilder.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <optional>

#include "domain/ports/calibration/calculation/ICalibrationCalculator.h"

using namespace application::orchestrators;
using namespace domain::common;

namespace {

std::optional<float> interpolateValueAtTime(
    const std::vector<float>& time,
    const std::vector<float>& values,
    float target_time)
{
    if (time.size() != values.size() || time.empty()) {
        return std::nullopt;
    }

    if (target_time < time.front() || target_time > time.back()) {
        return std::nullopt;
    }

    const auto upper = std::lower_bound(time.begin(), time.end(), target_time);
    if (upper == time.begin()) {
        return values.front();
    }
    if (upper == time.end()) {
        return values.back();
    }

    const std::size_t right_idx = static_cast<std::size_t>(std::distance(time.begin(), upper));
    const std::size_t left_idx = right_idx - 1;

    const float t0 = time[left_idx];
    const float t1 = time[right_idx];
    const float v0 = values[left_idx];
    const float v1 = values[right_idx];

    if (std::abs(t1 - t0) <= std::numeric_limits<float>::epsilon()) {
        return v0;
    }

    const float alpha = (target_time - t0) / (t1 - t0);
    return v0 + (v1 - v0) * alpha;
}

std::optional<float> interpolateAngleAtPressure(
    const PressureSeries& pressure_series,
    const AngleSeries& angle_series,
    float target_pressure)
{
    const auto& pressure_time = pressure_series.time();
    const auto& pressure_vals = pressure_series.value();
    const auto& angle_time = angle_series.time();
    if (pressure_time.empty() || angle_time.empty()) {
        return std::nullopt;
    }

    if (target_pressure < *std::min_element(pressure_vals.begin(), pressure_vals.end())
        || target_pressure > *std::max_element(pressure_vals.begin(), pressure_vals.end())) {
        return std::nullopt;
    }

    for (std::size_t i = 1; i < pressure_vals.size(); ++i) {
        const float p0 = pressure_vals[i - 1];
        const float p1 = pressure_vals[i];
        const bool crossed = (p0 <= target_pressure && target_pressure <= p1)
            || (p1 <= target_pressure && target_pressure <= p0);
        if (!crossed) {
            continue;
        }

        const float t0 = pressure_time[i - 1];
        const float t1 = pressure_time[i];
        float target_time = t0;
        if (std::abs(p1 - p0) > std::numeric_limits<float>::epsilon()) {
            const float alpha = (target_pressure - p0) / (p1 - p0);
            target_time = t0 + (t1 - t0) * alpha;
        }
        return interpolateValueAtTime(angle_time, angle_series.value(), target_time);
    }

    return std::nullopt;
}

} // namespace


CalibrationResultBuilder::CalibrationResultBuilder(CalibrationResultBuilderPorts ports)
    : logger_(ports.logger), ports_(ports)
{
    ports_.calibration_recorder.addObserver(*this);
    logger_.info("CalibrationResultBuilder constructor called");
}

CalibrationResultBuilder::~CalibrationResultBuilder() {
    ports_.calibration_recorder.removeObserver(*this);
    logger_.info("CalibrationResultBuilder destructor called");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

void CalibrationResultBuilder::onCalibrationRecorderEvent(const CalibrationRecorderEvent &ev) {
    std::visit(
    [this](const auto& e)
    {
        using T = std::decay_t<decltype(e)>;

        if constexpr (std::is_same_v<T, CalibrationRecorderEvent::RecordingStarted> ||
            std::is_same_v<T, CalibrationRecorderEvent::SessionEnded>)
        {
            handleEvent(e);
        }
    },
    ev.data);
}

void CalibrationResultBuilder::handleEvent(const CalibrationRecorderEvent::RecordingStarted &e) {
    logger_.info("RecordingStarted: points_count={} sources_count={} directions_count={}",
        e.layout.points.size(), e.layout.sources.size(), e.layout.directions.size());

    active_layout_ = e.layout;
    active_result_ = CalibrationResult(e.layout, e.gauge);

    observers_.notify([this] (domain::ports::ICalibrationResultObserver &o) {
        o.onCalibrationResultUpdated(*active_result_);
    });
}

void CalibrationResultBuilder::handleEvent(const CalibrationRecorderEvent::SessionEnded &e) {
    if (!active_result_ || !active_layout_) {
        logger_.warn("SessionEnded received but no active calibration session");
        return;
    }

    logger_.info(
        "SessionEnded: direction={}, pressure={}",
        e.id.direction,
        e.result.id.point.pressure
    );


    CalibrationCellKey key;
    key.direction = e.id.direction;
    key.point_id = e.id.point;

    for (const auto& source_id : active_layout_->sources) {
        // Оптимизируем поиск: ищем один раз
        auto it = e.result.angle_series.find(source_id);
        if (it == e.result.angle_series.end()) continue;

        key.source_id = source_id;

        logger_.info(
            "Processing camera {}: angle_samples={}, pressure_samples={}",
            source_id.value, it->second.size(), e.result.pressure_series.size());


        CalibrationCalculatorInput calculator_input {
            e.result.id.point.pressure,
            it->second, // Используем найденное значение
            e.result.pressure_series
        };

        const auto result = ports_.calibration_calculator.compute(calculator_input);
        active_result_->setCell(key, result.cell);

        const auto first_point = active_layout_->points.front();
        const auto last_point = active_layout_->points.back();
        const auto first_angle = active_result_->cell(CalibrationCellKey{first_point, source_id, e.id.direction});
        const auto last_angle = active_result_->cell(CalibrationCellKey{last_point, source_id, e.id.direction});
        const auto center_angle = interpolateAngleAtPressure(
            e.result.pressure_series,
            it->second,
            active_result_->gauge().central_pressure);

        if (first_angle && last_angle
            && first_angle->angle().has_value()
            && last_angle->angle().has_value()
            && center_angle.has_value())
        {
            const float expected_center = (*first_angle->angle() + *last_angle->angle()) * 0.5F;
            const float center_deviation = std::abs(*center_angle - expected_center);
            active_result_->setCenterDeviation(source_id, e.id.direction, center_deviation);
        }

        logger_.info(
            "Calibration cell updated: source={}, direction={}, pressure={}, angle={}, issues_count={}",
            key.source_id.value,
            key.direction,
            key.point_id.pressure,
            result.cell.angle(),
            result.cell.issues().size()
        );

        observers_.notify([this] (domain::ports::ICalibrationResultObserver &o) {
            o.onCalibrationResultUpdated(*active_result_);
        });
    }
}

void CalibrationResultBuilder::handleEvent(const CalibrationRecorderEvent::RecordingStopped &e) {
    if (!active_result_) return;
    active_result_->markReady();
    observers_.notify([this] (domain::ports::ICalibrationResultObserver &o) {
        o.onCalibrationResultUpdated(*active_result_);
    });
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

const std::optional<CalibrationResult>& CalibrationResultBuilder::currentResult() const {
    return active_result_;
}

void CalibrationResultBuilder::addObserver(domain::ports::ICalibrationResultObserver &o) {
    observers_.add(o);
}

void CalibrationResultBuilder::removeObserver(domain::ports::ICalibrationResultObserver &o) {
    observers_.remove(o);
}
