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

void CalibrationSessionControlViewModel::startCalibration() {
    const auto mode = backward_stroke_enabled.get_copy()
        ? domain::common::CalibrationMode::OnlyBackward
        : domain::common::CalibrationMode::Full;
    startWithMode(mode);
}

void CalibrationSessionControlViewModel::startAim()
{
    startWithMode(domain::common::CalibrationMode::OnlyLast);
}

void CalibrationSessionControlViewModel::startWithMode(domain::common::CalibrationMode mode)
{
    applyState(CalibrationOrchestratorState::Starting, "");
    status_text.set("Подготовка сценария градуировки");
    std::string last_error;

    if (!control_.start(mode,
                        slowdown_enabled.get_copy(),
                        play_valve_enabled.get_copy(),
                        last_error)) {
        applyState(CalibrationOrchestratorState::Stopped, last_error);
        status_text.set("Ошибка запуска градуировки");
    }
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

void CalibrationSessionControlViewModel::setBackwardStrokeEnabled(bool enabled)
{
    backward_stroke_enabled.set(enabled);
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
    status_text.set("Экстренный останов двигателя");
    control_.emergencyStop();
}

void CalibrationSessionControlViewModel::onCalibrationOrchestratorEvent(
    const CalibrationOrchestratorEvent& ev)
{
    std::visit([this](const auto& event) {
        using T = std::decay_t<decltype(event)>;

        if constexpr (std::is_same_v<T, CalibrationOrchestratorEvent::Started>) {
            applyState(CalibrationOrchestratorState::Started, "");
            status_text.set("Градуировка выполняется");
        }
        else if constexpr (std::is_same_v<T, CalibrationOrchestratorEvent::Stopped>) {
            applyState(CalibrationOrchestratorState::Stopped, "");
            status_text.set("Градуировка остановлена");
        }
        else if constexpr (std::is_same_v<T, CalibrationOrchestratorEvent::Failed>) {
            applyState(CalibrationOrchestratorState::Stopped, event.error);
            status_text.set("Сценарий завершён с ошибкой");
        }
        else if constexpr (std::is_same_v<T, CalibrationOrchestratorEvent::StatusChanged>) {
            status_text.set(event.message);
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
