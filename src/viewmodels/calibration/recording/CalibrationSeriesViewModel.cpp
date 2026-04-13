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
    static const std::vector<AngleEntry> empty;

    const auto it = angle_history_.find(source_id);
    if (it == angle_history_.end()) {
        return empty;
    }

    return it->second;
}

const std::vector<SourceId> & CalibrationSeriesViewModel::openedSources() const {
    return deps_.video_source_manager.opened();
}

void CalibrationSeriesViewModel::onCalibrationRecorderEvent(
    const CalibrationRecorderEvent& ev)
{
    if (!source_ids.has_observers()) return;
    std::visit(
        [this](const auto& e)
        {
            using T = std::decay_t<decltype(e)>;

            if constexpr (std::is_same_v<T,CalibrationRecorderEvent::RecordingStarted>)
            {
                source_ids.set(deps_.video_source_manager.opened());
                pressure_history_.clear();
                current_pressure.set({});
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
                current_angle.set({e.sample.id, current});
            }
            else if (std::is_same_v<T,CalibrationRecorderEvent::SessionStarted>) {
                in_session_ = true;
            }
            else if (std::is_same_v<T,CalibrationRecorderEvent::SessionEnded>) {
                in_session_ = false;
            }
        },
        ev.data);
}
