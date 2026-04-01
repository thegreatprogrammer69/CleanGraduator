#include "CalibrationResultTableViewModel.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <optional>
#include <type_traits>
#include <vector>

#include "domain/ports/calibration/recording/ICalibrationRecorder.h"
#include "domain/ports/calibration/result/ICalibrationResultSource.h"
#include "domain/ports/calibration/result/ICalibrationResultValidationSource.h"
#include "application/ports/catalogs/IGaugeCatalog.h"

using namespace mvvm;
using namespace domain::common;

namespace {

std::optional<float> angleFor(const CalibrationResult& result,
                              SourceId source_id,
                              PointId point_id,
                              MotorDirection direction)
{
    CalibrationCellKey key;
    key.source_id = source_id;
    key.point_id = point_id;
    key.direction = direction;

    const auto cell = result.cell(key);
    if (!cell || !cell->angle()) {
        return std::nullopt;
    }

    return cell->angle();
}

} // namespace

CalibrationResultTableViewModel::CalibrationResultTableViewModel(CalibrationResultTableViewModelDeps deps)
    : result_source_(deps.result_source)
    , validation_source_(deps.validation_source)
    , recorder_(deps.recorder)
    , settings_storage_(deps.settings_storage)
    , gauge_catalog_(deps.gauge_catalog)
{
    result_source_.addObserver(*this);
    validation_source_.addObserver(*this);
    recorder_.addObserver(*this);

    current_result.set(result_source_.currentResult());
    current_validation.set(validation_source_.currentValidation());

    resetInfo();
    refreshSettings();
    refreshTemplatePressures();
    refreshMeasurementCountsFromRecorder();
    if (const auto& result = result_source_.currentResult();
        result && isCurrentResultCompatibleWithSelectedGauge()) {
        updateInfoFromResult(*result);
    } else {
        current_result.set(std::nullopt);
    }
    updateCurrentInfo();
}

CalibrationResultTableViewModel::~CalibrationResultTableViewModel() {
    recorder_.removeObserver(*this);
    validation_source_.removeObserver(*this);
    result_source_.removeObserver(*this);
}

void CalibrationResultTableViewModel::onCalibrationResultUpdated(const CalibrationResult &result) {
    current_result.set(result);
    if (!isCurrentResultCompatibleWithSelectedGauge()) {
        current_result.set(std::nullopt);
        resetInfo();
        refreshSettings();
        refreshTemplatePressures();
        refreshMeasurementCountsFromRecorder();
        updateCurrentInfo();
        return;
    }

    updateInfoFromResult(result);
}

void CalibrationResultTableViewModel::onCalibrationResultValidationUpdated(const CalibrationResultValidation& validation) {
    current_validation.set(validation, true);
    refreshSettings();
    refreshTemplatePressures();
    if (!isCurrentResultCompatibleWithSelectedGauge()) {
        current_result.set(std::nullopt);
        resetInfo();
        refreshSettings();
        refreshTemplatePressures();
        refreshMeasurementCountsFromRecorder();
        updateCurrentInfo();
        return;
    }
    updateCurrentInfo();
}

void CalibrationResultTableViewModel::onCalibrationRecorderEvent(const CalibrationRecorderEvent& ev)
{
    std::visit([this](const auto& e) {
        using T = std::decay_t<decltype(e)>;

        if constexpr (std::is_same_v<T, CalibrationRecorderEvent::RecordingStarted>) {
            resetInfo();
            refreshMeasurementCountsFromRecorder();
            updateCurrentInfo();
        } else if constexpr (std::is_same_v<T, CalibrationRecorderEvent::AngleSampleRecorded>) {
            info_.current_angles[e.sample.id] = e.sample.angle;
            refreshMeasurementCountsFromRecorder();
            updateCurrentInfo();
        } else if constexpr (std::is_same_v<T, CalibrationRecorderEvent::SessionStarted>
            || std::is_same_v<T, CalibrationRecorderEvent::SessionEnded>
            || std::is_same_v<T, CalibrationRecorderEvent::RecordingStopped>) {
            refreshMeasurementCountsFromRecorder();
            updateCurrentInfo();
        }
    }, ev.data);
}

void CalibrationResultTableViewModel::resetInfo()
{
    info_ = {};
}

void CalibrationResultTableViewModel::updateInfoFromResult(const CalibrationResult& result)
{
    for (const auto& source_id : result.sources()) {
        if (!result.points().empty()) {
            const auto& first_point = result.points().front();
            const auto& last_point = result.points().back();
            const auto first_angle = angleFor(result, source_id, first_point, MotorDirection::Forward);
            const auto last_angle = angleFor(result, source_id, last_point, MotorDirection::Forward);
            if (first_angle && last_angle) {
                info_.total_angles[source_id] = *last_angle - *first_angle;
            }
        }

        for (const auto direction : result.directions()) {
            const auto nonlinearity = calculateNonlinearity(result, source_id, direction);
            if (nonlinearity) {
                info_.nonlinearities[source_id][direction] = *nonlinearity;
            }
            const auto center_deviation = calculateCenterDeviationDeg(result, source_id, direction);
            if (center_deviation) {
                info_.center_deviations_deg[source_id][direction] = *center_deviation;
            }
        }
    }

    updateCurrentInfo();
}

void CalibrationResultTableViewModel::updateCurrentInfo()
{
    current_info.set(info_, true);
}

void CalibrationResultTableViewModel::refreshMeasurementCountsFromRecorder()
{
    info_.measurement_counts = recorder_.angleCounts();
}

std::optional<float> CalibrationResultTableViewModel::calculateNonlinearity(
    const CalibrationResult& result,
    SourceId source_id,
    MotorDirection direction)
{
    std::vector<float> angles;
    angles.reserve(result.points().size());

    for (const auto& point : result.points()) {
        const auto angle = angleFor(result, source_id, point, direction);
        if (!angle) {
            return std::nullopt;
        }
        angles.push_back(*angle);
    }

    if (angles.size() < 2) {
        return std::nullopt;
    }

    const float avrDelta = (angles.back() - angles.front()) / static_cast<float>(angles.size() - 1);
    if (std::abs(avrDelta) <= std::numeric_limits<float>::epsilon()) {
        return std::nullopt;
    }

    float maxD = 0.0f;
    for (std::size_t i = 0; i + 1 < angles.size(); ++i) {
        const float delta = angles[i + 1] - angles[i];
        maxD = std::max(maxD, std::abs(delta - avrDelta));
    }

    return (maxD / std::abs(avrDelta)) * 100.0f;
}

std::optional<float> CalibrationResultTableViewModel::calculateCenterDeviationDeg(
    const CalibrationResult& result,
    SourceId source_id,
    MotorDirection direction)
{
    const auto& points = result.points();
    if (points.size() < 2) {
        return std::nullopt;
    }

    const auto first_angle = angleFor(result, source_id, points.front(), direction);
    const auto last_angle = angleFor(result, source_id, points.back(), direction);
    if (!first_angle || !last_angle) {
        return std::nullopt;
    }

    const float central_pressure = result.gauge().central_pressure;
    if (central_pressure < points.front().pressure || central_pressure > points.back().pressure) {
        return std::nullopt;
    }

    for (std::size_t i = 0; i + 1 < points.size(); ++i) {
        const auto& left = points[i];
        const auto& right = points[i + 1];
        if (central_pressure < left.pressure || central_pressure > right.pressure) {
            continue;
        }

        const auto left_angle = angleFor(result, source_id, left, direction);
        const auto right_angle = angleFor(result, source_id, right, direction);
        if (!left_angle || !right_angle) {
            return std::nullopt;
        }

        const float pressure_span = right.pressure - left.pressure;
        if (std::abs(pressure_span) <= std::numeric_limits<float>::epsilon()) {
            return std::nullopt;
        }

        const float alpha = (central_pressure - left.pressure) / pressure_span;
        const float central_angle = *left_angle + alpha * (*right_angle - *left_angle);
        const float full_span = *last_angle - *first_angle;
        const float expected_center_angle = *first_angle + full_span * 0.5F;
        return std::abs(central_angle - expected_center_angle);
    }

    return std::nullopt;
}

void CalibrationResultTableViewModel::refreshSettings()
{
    const auto settings = settings_storage_.loadInfoSettings();
    info_.centered_mark_enabled = settings.centered_mark_enabled;
    info_.max_center_deviation_deg = settings.max_center_deviation_deg;
}

void CalibrationResultTableViewModel::refreshTemplatePressures()
{
    info_.template_pressures.clear();

    const auto settings = settings_storage_.loadInfoSettings();
    const auto gauge = gauge_catalog_.at(settings.gauge_idx);
    if (!gauge) {
        return;
    }

    info_.template_pressures.reserve(gauge->points.value.size());
    for (const auto pressure_point : gauge->points.value) {
        info_.template_pressures.push_back(pressure_point);
    }
}

bool CalibrationResultTableViewModel::isCurrentResultCompatibleWithSelectedGauge() const
{
    const auto result = current_result.get_copy();
    if (!result) {
        return true;
    }

    const auto settings = settings_storage_.loadInfoSettings();
    const auto gauge = gauge_catalog_.at(settings.gauge_idx);
    if (!gauge) {
        return false;
    }

    const auto& result_points = result->points();
    const auto& gauge_points = gauge->points.value;
    if (result_points.size() != gauge_points.size()) {
        return false;
    }

    for (std::size_t index = 0; index < gauge_points.size(); ++index) {
        if (result_points[index].pressure != gauge_points[index]) {
            return false;
        }
    }

    return true;
}
