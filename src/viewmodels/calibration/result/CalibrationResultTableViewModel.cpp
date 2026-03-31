#include "CalibrationResultTableViewModel.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <cstdlib>
#include <optional>
#include <vector>
#include <type_traits>

#include "domain/ports/calibration/recording/ICalibrationRecorder.h"
#include "domain/ports/calibration/result/ICalibrationResultSource.h"
#include "domain/ports/calibration/result/ICalibrationResultValidationSource.h"

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
{
    result_source_.addObserver(*this);
    validation_source_.addObserver(*this);
    recorder_.addObserver(*this);

    current_result.set(result_source_.currentResult());
    current_validation.set(validation_source_.currentValidation());

    resetInfo();
    refreshMeasurementCountsFromRecorder();
    if (const auto& result = result_source_.currentResult(); result) {
        updateInfoFromResult(*result);
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
    updateInfoFromResult(result);
}

void CalibrationResultTableViewModel::onCalibrationResultValidationUpdated(const CalibrationResultValidation& validation) {
    current_validation.set(validation, true);
    const auto settings = settings_storage_.loadInfoSettings();
    info_.centered_mark_enabled = settings.centered_mark_enabled;
    info_.max_center_deviation_deg = settings.max_center_deviation_deg;
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
    const auto settings = settings_storage_.loadInfoSettings();
    info_.centered_mark_enabled = settings.centered_mark_enabled;
    info_.max_center_deviation_deg = settings.max_center_deviation_deg;
}

void CalibrationResultTableViewModel::updateInfoFromResult(const CalibrationResult& result)
{
    const auto settings = settings_storage_.loadInfoSettings();
    info_.centered_mark_enabled = settings.centered_mark_enabled;
    info_.max_center_deviation_deg = settings.max_center_deviation_deg;

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

            const auto centered_mark_deviation = calculateCenteredMarkDeviation(result, source_id, direction);
            if (centered_mark_deviation) {
                info_.centered_mark_deviations_deg[source_id][direction] = *centered_mark_deviation;
            }
        }
    }

    updateCurrentInfo();
}

std::optional<float> CalibrationResultTableViewModel::calculateCenteredMarkDeviation(
    const CalibrationResult& result,
    SourceId source_id,
    MotorDirection direction)
{
    if (result.points().size() < 2) {
        return std::nullopt;
    }

    const auto first_angle = angleFor(result, source_id, result.points().front(), direction);
    const auto last_angle = angleFor(result, source_id, result.points().back(), direction);
    if (!first_angle || !last_angle) {
        return std::nullopt;
    }

    struct Sample {
        float pressure;
        float angle;
    };
    std::vector<Sample> samples;
    samples.reserve(result.points().size());

    for (const auto& point : result.points()) {
        const auto point_angle = angleFor(result, source_id, point, direction);
        if (point_angle) {
            samples.push_back({point.pressure, *point_angle});
        }
    }

    if (samples.size() < 2) {
        return std::nullopt;
    }

    const float center_pressure = result.gauge().central_pressure;
    std::sort(samples.begin(), samples.end(), [](const Sample& lhs, const Sample& rhs) {
        return lhs.pressure < rhs.pressure;
    });

    auto center_it = std::find_if(samples.begin(), samples.end(), [center_pressure](const Sample& sample) {
        return std::abs(sample.pressure - center_pressure) < std::numeric_limits<float>::epsilon();
    });

    float center_angle = 0.0F;
    if (center_it != samples.end()) {
        center_angle = center_it->angle;
    } else {
        auto right_it = std::find_if(samples.begin(), samples.end(), [center_pressure](const Sample& sample) {
            return sample.pressure > center_pressure;
        });
        if (right_it == samples.begin() || right_it == samples.end()) {
            return std::nullopt;
        }
        const auto left_it = std::prev(right_it);
        const float pressure_span = right_it->pressure - left_it->pressure;
        if (std::abs(pressure_span) < std::numeric_limits<float>::epsilon()) {
            return std::nullopt;
        }

        const float t = (center_pressure - left_it->pressure) / pressure_span;
        center_angle = left_it->angle + (right_it->angle - left_it->angle) * t;
    }

    const float total_span_angle = *last_angle - *first_angle;
    const float center_span_angle = center_angle - *first_angle;
    return std::abs(center_span_angle - total_span_angle * 0.5F);
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
