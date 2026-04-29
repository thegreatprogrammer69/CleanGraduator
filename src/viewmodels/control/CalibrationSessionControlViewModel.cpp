#include "CalibrationSessionControlViewModel.h"

#include <algorithm>
#include <type_traits>
#include <variant>

#include "application/orchestrators/calibration/process/CalibrationOrchestratorEvent.h"

using namespace mvvm;
using namespace application::orchestrators;

CalibrationSessionControlViewModel::CalibrationSessionControlViewModel(
    CalibrationSessionControlViewModelDeps deps)
    : control_(deps.control)
    , settings_storage_(deps.settings_storage)
    , validation_source_(deps.validation_source)
{
    control_.addObserver(*this);

    const auto initial_state = control_.isRunning()
        ? CalibrationOrchestratorState::Started
        : CalibrationOrchestratorState::Stopped;

    const auto settings = settings_storage_.loadInfoSettings();
    ku_mode_enabled.set(settings.ku_enabled);
    centered_mark_enabled.set(settings.centered_mark_enabled);
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


void CalibrationSessionControlViewModel::setKuModeEnabled(bool enabled)
{
    ku_mode_enabled.set(enabled);
    auto data = settings_storage_.loadInfoSettings();
    data.ku_enabled = enabled;
    settings_storage_.saveInfoSettings(data);
    validation_source_.requestRefresh();
}

void CalibrationSessionControlViewModel::setCenteredMarkEnabled(bool enabled)
{
    centered_mark_enabled.set(enabled);
    auto data = settings_storage_.loadInfoSettings();
    data.centered_mark_enabled = enabled;
    settings_storage_.saveInfoSettings(data);
    validation_source_.requestRefresh();
}

void CalibrationSessionControlViewModel::startCalibration() {
    const auto mode = reverse_mode_enabled.get_copy()
        ? domain::common::CalibrationMode::OnlyForward
        : domain::common::CalibrationMode::Full;

    selected_mode.set(mode);
    applyState(CalibrationOrchestratorState::Starting, "");
    status_text.set("Подготовка к запуску");
    resetProgress();

    std::string error_text;
    if (!control_.start(mode, slowdown_enabled.get_copy(), play_valve_enabled.get_copy(), error_text)) {
        applyState(CalibrationOrchestratorState::Stopped, error_text);
        status_text.set("Ошибка запуска");
    }
}

void CalibrationSessionControlViewModel::aimCalibration() {
    selected_mode.set(domain::common::CalibrationMode::OnlyLast);
    applyState(CalibrationOrchestratorState::Starting, "");
    status_text.set("Запуск прицела: последняя точка");
    resetProgress();

    std::string error_text;
    if (!control_.start(domain::common::CalibrationMode::OnlyLast, slowdown_enabled.get_copy(), play_valve_enabled.get_copy(), error_text)) {
        applyState(CalibrationOrchestratorState::Stopped, error_text);
        status_text.set("Ошибка запуска прицела");
    }
}

void CalibrationSessionControlViewModel::setReverseModeEnabled(bool enabled)
{
    reverse_mode_enabled.set(enabled);
}

void CalibrationSessionControlViewModel::setSlowdownEnabled(bool enabled)
{
    slowdown_enabled.set(enabled);
}

void CalibrationSessionControlViewModel::setPlayValveEnabled(bool enabled)
{
    play_valve_enabled.set(enabled);
}

void CalibrationSessionControlViewModel::stopCalibration() {
    applyState(CalibrationOrchestratorState::Stopping, "");
    status_text.set("Остановка двигателя");
    control_.stop();
}

void CalibrationSessionControlViewModel::emergencyStop() {
    applyState(CalibrationOrchestratorState::Stopping, "");
    status_text.set("Экстренный стоп");
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
            resetProgress();
        }
        else if constexpr (std::is_same_v<T, CalibrationOrchestratorEvent::Failed>) {
            applyState(CalibrationOrchestratorState::Stopped, event.error);
            status_text.set("Ошибка: " + event.error);
            resetProgress();
        }
        else if constexpr (std::is_same_v<T, CalibrationOrchestratorEvent::StatusText>) {
            status_text.set(event.text);
        }
        else if constexpr (std::is_same_v<T, CalibrationOrchestratorEvent::Progress>) {
            forward_progress_percent.set(std::clamp(event.forward_percent, 0, 100));
            backward_progress_percent.set(std::clamp(event.backward_percent, 0, 100));
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

    if (state == CalibrationOrchestratorState::Stopped && last_error.empty()) {
        status_text.set("Ожидание запуска");
    }
}

void CalibrationSessionControlViewModel::resetProgress()
{
    forward_progress_percent.set(0);
    backward_progress_percent.set(0);
}
