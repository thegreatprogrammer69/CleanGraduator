#include "CalibrationSessionControlViewModel.h"

#include <type_traits>
#include <variant>

#include "application/orchestrators/calibration/process/CalibrationOrchestratorEvent.h"

using namespace mvvm;
using namespace application::orchestrators;

CalibrationSessionControlViewModel::CalibrationSessionControlViewModel(
    CalibrationSessionControlViewModelDeps deps)
    : orchestrator_(deps.orchestrator)
    , settings_query_(deps.settings_query)
{
    orchestrator_.addObserver(*this);

    const auto initial_state = orchestrator_.isRunning()
        ? CalibrationOrchestratorState::Started
        : CalibrationOrchestratorState::Stopped;

    applyState(initial_state, "");
}

CalibrationSessionControlViewModel::~CalibrationSessionControlViewModel() {
    orchestrator_.removeObserver(*this);
}

void CalibrationSessionControlViewModel::setCalibrationMode(
    domain::common::CalibrationMode mode)
{
    selected_mode.set(mode);
}

void CalibrationSessionControlViewModel::startCalibration() {
    applyState(CalibrationOrchestratorState::Starting, "");

    settings_query_.load();

    const auto gauge = settings_query_.currentGauge();
    const auto pressure_unit   = settings_query_.currentPressureUnit();

    if (!gauge || !gauge->points.isCorrect() || !pressure_unit) {
        applyState(
            CalibrationOrchestratorState::Stopped,
            "Не удалось запустить: проверьте настройки шкалы и единиц давления.");
        return;
    }

    CalibrationOrchestratorInput input{
        selected_mode.get_copy(),
        *pressure_unit,
        domain::common::AngleUnit::deg,
        *gauge
    };

    if (!orchestrator_.start(input)) {
        applyState(
            CalibrationOrchestratorState::Stopped,
            "Не удалось запустить процесс калибровки.");
    }
}

void CalibrationSessionControlViewModel::stopCalibration() {
    applyState(CalibrationOrchestratorState::Stopping, "");
    orchestrator_.stop();
}

void CalibrationSessionControlViewModel::emergencyStop() {
    // Можно оставить так или выделить отдельное состояние при желании
    applyState(CalibrationOrchestratorState::Stopping, "");
    orchestrator_.stop();
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