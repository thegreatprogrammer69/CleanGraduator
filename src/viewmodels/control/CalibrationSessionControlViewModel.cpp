#include "CalibrationSessionControlViewModel.h"

#include <type_traits>
#include <variant>

#include "application/orchestrators/calibration/process/CalibrationOrchestratorEvent.h"

using namespace mvvm;

CalibrationSessionControlViewModel::CalibrationSessionControlViewModel(CalibrationSessionControlViewModelDeps deps)
    : orchestrator_(deps.orchestrator)
    , settings_query_(deps.settings_query)
{
    orchestrator_.addObserver(*this);
    applyState(
        orchestrator_.isRunning()
            ? application::orchestrators::CalibrationOrchestratorState::Started
            : application::orchestrators::CalibrationOrchestratorState::Stopped,
        "");
}

CalibrationSessionControlViewModel::~CalibrationSessionControlViewModel() {
    orchestrator_.removeObserver(*this);
}

void CalibrationSessionControlViewModel::setCalibrationMode(domain::common::CalibrationMode mode) {
    selected_mode.set(mode);
}

void CalibrationSessionControlViewModel::startCalibration() {
    settings_query_.load();
    const auto pressure_points = settings_query_.currentGaugePressurePoints();
    const auto pressure_unit = settings_query_.currentPressureUnit();

    if (!pressure_points || !pressure_points->isCorrect() || !pressure_unit) {
        applyState(
            application::orchestrators::CalibrationOrchestratorState::Stopped,
            "Не удалось запустить: проверьте настройки шкалы и единиц давления.");
        return;
    }

    application::orchestrators::CalibrationOrchestratorInput input{
        selected_mode.get_copy(),
        *pressure_unit,
        domain::common::AngleUnit::deg,
        *pressure_points
    };

    if (!orchestrator_.start(input)) {
        applyState(
            application::orchestrators::CalibrationOrchestratorState::Stopped,
            "Не удалось запустить процесс калибровки.");
    }
}

void CalibrationSessionControlViewModel::stopCalibration() {
    orchestrator_.stop();
}

void CalibrationSessionControlViewModel::emergencyStop() {
    orchestrator_.stop();
}

void CalibrationSessionControlViewModel::onCalibrationOrchestratorEvent(
    const application::orchestrators::CalibrationOrchestratorEvent &ev) {
    std::visit([this](const auto& event) {
        using T = std::decay_t<decltype(event)>;

        if constexpr (std::is_same_v<T, application::orchestrators::CalibrationOrchestratorEvent::Started>) {
            applyState(application::orchestrators::CalibrationOrchestratorState::Started, "");
        } else if constexpr (std::is_same_v<T, application::orchestrators::CalibrationOrchestratorEvent::Stopped>) {
            applyState(application::orchestrators::CalibrationOrchestratorState::Stopped, "");
        } else if constexpr (std::is_same_v<T, application::orchestrators::CalibrationOrchestratorEvent::Failed>) {
            applyState(application::orchestrators::CalibrationOrchestratorState::Stopped, event.error);
        }
    }, ev.data);
}

void CalibrationSessionControlViewModel::applyState(
    application::orchestrators::CalibrationOrchestratorState state,
    const std::string& last_error)
{
    error_text.set(last_error);

    const bool can_start_now = state == application::orchestrators::CalibrationOrchestratorState::Stopped;
    const bool can_stop_now =
        state == application::orchestrators::CalibrationOrchestratorState::Starting
        || state == application::orchestrators::CalibrationOrchestratorState::Started
        || state == application::orchestrators::CalibrationOrchestratorState::Stopping;
    const bool can_abort_now = state == application::orchestrators::CalibrationOrchestratorState::Started;

    can_start.set(can_start_now);
    can_stop.set(can_stop_now);
    can_abort.set(can_abort_now);
}
