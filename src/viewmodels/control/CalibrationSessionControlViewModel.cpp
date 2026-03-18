#include "CalibrationSessionControlViewModel.h"

#include <type_traits>
#include <variant>

#include "application/orchestrators/calibration/process/CalibrationOrchestratorEvent.h"

using namespace mvvm;
using namespace application::orchestrators;

CalibrationSessionControlViewModel::CalibrationSessionControlViewModel(
    CalibrationSessionControlViewModelDeps deps)
    : control_(deps.control)
{
    control_.addObserver(*this);

    const auto initial_state = control_.isRunning()
        ? CalibrationOrchestratorState::Started
        : CalibrationOrchestratorState::Stopped;

    applyState(initial_state, "");
}

CalibrationSessionControlViewModel::~CalibrationSessionControlViewModel() {
    control_.removeObserver(*this);
}

void CalibrationSessionControlViewModel::setCalibrationMode(
    domain::common::CalibrationMode mode)
{
    selected_mode.set(mode);
}

void CalibrationSessionControlViewModel::startCalibration() {
    applyState(CalibrationOrchestratorState::Starting, "");
    std::string error_text;
    if (!control_.start(selected_mode.get_copy(), error_text)) {
        applyState(CalibrationOrchestratorState::Stopped, error_text);
    }
}

void CalibrationSessionControlViewModel::stopCalibration() {
    applyState(CalibrationOrchestratorState::Stopping, "");
    control_.stop();
}

void CalibrationSessionControlViewModel::emergencyStop() {
    // Можно оставить так или выделить отдельное состояние при желании
    applyState(CalibrationOrchestratorState::Stopping, "");
    control_.emergencyStop();
}

void CalibrationSessionControlViewModel::onCalibrationOrchestratorEvent(
    const CalibrationOrchestratorEvent& ev)
{
    std::visit([this](const auto& event) {
        using T = std::decay_t<decltype(event)>;

        if constexpr (std::is_same_v<T, CalibrationOrchestratorEvent::Started>) {
            applyState(CalibrationOrchestratorState::Started, "");
        }
        else if constexpr (std::is_same_v<T, CalibrationOrchestratorEvent::Stopped>) {
            applyState(CalibrationOrchestratorState::Stopped, "");
        }
        else if constexpr (std::is_same_v<T, CalibrationOrchestratorEvent::Failed>) {
            applyState(CalibrationOrchestratorState::Stopped, event.error);
        }
    }, ev.data);
}

void CalibrationSessionControlViewModel::applyState(
    CalibrationOrchestratorState state,
    const std::string& last_error)
{
    error_text.set(last_error);

    const bool is_stopped  = state == CalibrationOrchestratorState::Stopped;
    const bool is_active   =
        state == CalibrationOrchestratorState::Starting ||
        state == CalibrationOrchestratorState::Started  ||
        state == CalibrationOrchestratorState::Stopping;

    const bool is_started  = state == CalibrationOrchestratorState::Started;

    can_start.set(is_stopped);
    can_stop.set(is_active);
    can_abort.set(is_started);
}
