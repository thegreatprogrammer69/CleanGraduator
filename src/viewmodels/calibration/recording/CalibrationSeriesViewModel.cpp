#include "CalibrationSeriesViewModel.h"

#include <variant>

#include "application/orchestrators/video/VideoSourceManager.h"

using namespace mvvm;
using namespace domain::common;
using namespace domain::ports;

CalibrationSeriesViewModel::CalibrationSeriesViewModel(CalibrationSeriesViewModelDeps deps)
    : deps_(deps)
{
    deps_.recorder.addObserver(*this);
}

CalibrationSeriesViewModel::~CalibrationSeriesViewModel()
{
    deps_.recorder.removeObserver(*this);
}

const std::vector<CalibrationSeriesViewModel::PressureEntry>&
CalibrationSeriesViewModel::pressureHistory() const
{
    return pressure_history_;
}

const std::vector<CalibrationSeriesViewModel::AngleEntry>&
CalibrationSeriesViewModel::angleHistory(SourceId source_id) const
{
    if (angle_history_.find(source_id) == angle_history_.end()) return {};
    return angle_history_.at(source_id);
}

const std::vector<SourceId> & CalibrationSeriesViewModel::openedSources() const {
    return deps_.video_source_manager.opened();
}

int CalibrationSeriesViewModel::angleMeasurementCount(SourceId source_id, MotorDirection direction) const
{
    const auto source_it = angle_measurement_count_.find(source_id);
    if (source_it == angle_measurement_count_.end()) return 0;

    const auto direction_it = source_it->second.find(direction);
    if (direction_it == source_it->second.end()) return 0;

    return direction_it->second;
}

std::optional<float> CalibrationSeriesViewModel::currentAngle(SourceId source_id) const
{
    const auto it = current_angles_.find(source_id);
    if (it == current_angles_.end()) return std::nullopt;
    return it->second;
}

void CalibrationSeriesViewModel::onCalibrationRecorderEvent(
    const CalibrationRecorderEvent& ev)
{
    std::visit(
        [this](const auto& e)
        {
            using T = std::decay_t<decltype(e)>;

            if constexpr (std::is_same_v<T,CalibrationRecorderEvent::RecordingStarted>)
            {
                source_ids.set(deps_.video_source_manager.opened());
                pressure_history_.clear();
                angle_history_.clear();
                angle_measurement_count_.clear();
                current_angles_.clear();
                current_session_direction_.reset();
                current_pressure.set({});
                revision.set(++revision_counter_);
            }
            else if constexpr (std::is_same_v<T,CalibrationRecorderEvent::PressureSampleRecorded>)
            {
                auto current = PressureEntry{e.sample.time, e.sample.pressure, in_session_};
                pressure_history_.push_back(current);
                current_pressure.set(current);
            }
            else if constexpr (std::is_same_v<T,CalibrationRecorderEvent::AngleSampleRecorded>)
            {
                auto current = AngleEntry{e.sample.time, e.sample.angle, in_session_};
                angle_history_[e.sample.id].push_back(current);
                current_angles_[e.sample.id] = current.angle;
                if (current_session_direction_) {
                    angle_measurement_count_[e.sample.id][*current_session_direction_] += 1;
                }
                current_angle.set({e.sample.id, current});
                revision.set(++revision_counter_);
            }
            else if (std::is_same_v<T,CalibrationRecorderEvent::SessionStarted>) {
                in_session_ = true;
                current_session_direction_ = e.id.direction;
                revision.set(++revision_counter_);
            }
            else if (std::is_same_v<T,CalibrationRecorderEvent::SessionEnded>) {
                in_session_ = false;
                current_session_direction_.reset();
                revision.set(++revision_counter_);
            }
        },
        ev.data);
}
