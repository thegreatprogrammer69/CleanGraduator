#include "CalibrationResultTableViewModel.h"

#include <algorithm>
#include <cmath>
#include <limits>
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
    info_.measurement_counts.clear();

    for (const auto& session_id : recorder_.sessions()) {
        const auto session = recorder_.session(session_id);
        if (!session) {
            continue;
        }

        for (const auto& [source_id, series] : session->angle_series) {
            info_.measurement_counts[source_id][session_id.direction] +=
                static_cast<int>(series.size());
        }
    }
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
