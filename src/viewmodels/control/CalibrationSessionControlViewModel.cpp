#include "CalibrationSessionControlViewModel.h"

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

    std::string error_text;
    if (!control_.start(mode, slowdown_enabled.get_copy(), play_valve_enabled.get_copy(), error_text)) {
        emitSoundCue(SoundCue::ProcessError);
        applyState(CalibrationOrchestratorState::Stopped, error_text);
        status_text.set("Ошибка запуска");
    }
}

void CalibrationSessionControlViewModel::aimCalibration() {
    selected_mode.set(domain::common::CalibrationMode::OnlyLast);
    applyState(CalibrationOrchestratorState::Starting, "");
    status_text.set("Запуск прицела: последняя точка");

    std::string error_text;
    if (!control_.start(domain::common::CalibrationMode::OnlyLast, slowdown_enabled.get_copy(), play_valve_enabled.get_copy(), error_text)) {
        emitSoundCue(SoundCue::ProcessError);
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
            if (backward_phase_active_) {
                emitSoundCue(SoundCue::BackwardMovementFinished);
            }
            forward_phase_active_ = false;
            backward_phase_active_ = false;
            applyState(CalibrationOrchestratorState::Stopped, "");
        }
        else if constexpr (std::is_same_v<T, CalibrationOrchestratorEvent::Failed>) {
            emitSoundCue(SoundCue::ProcessError);
            forward_phase_active_ = false;
            backward_phase_active_ = false;
            applyState(CalibrationOrchestratorState::Stopped, event.error);
            status_text.set("Ошибка: " + event.error);
        }
        else if constexpr (std::is_same_v<T, CalibrationOrchestratorEvent::StatusText>) {
            handleStatusText(event.text);
            status_text.set(event.text);
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

void CalibrationSessionControlViewModel::handleStatusText(const std::string& text)
{
    const bool is_forward = text.rfind("Прямой ход:", 0) == 0;
    const bool is_backward = text.rfind("Обратный ход:", 0) == 0;

    if (forward_phase_active_ && is_backward) {
        emitSoundCue(SoundCue::ForwardMovementFinished);
    }

    forward_phase_active_ = is_forward;
    backward_phase_active_ = is_backward;
}

void CalibrationSessionControlViewModel::emitSoundCue(SoundCue cue)
{
    sound_cue.set(SoundCue::None);
    sound_cue.set(cue);
}
