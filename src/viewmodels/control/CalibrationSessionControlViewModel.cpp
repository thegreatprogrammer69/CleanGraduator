#include "CalibrationSessionControlViewModel.h"

#include <type_traits>
#include <variant>

#include "application/orchestrators/calibration/process/CalibrationOrchestratorInput.h"

using namespace mvvm;

CalibrationSessionControlViewModel::CalibrationSessionControlViewModel(CalibrationSessionControlViewModelDeps deps)
    : orchestrator_(deps.orchestrator)
    , settings_query_(deps.settings_query)
{
    orchestrator_.addObserver(*this);
    syncState(application::orchestrators::CalibrationOrchestratorState::Stopped);
}

CalibrationSessionControlViewModel::~CalibrationSessionControlViewModel() {
    orchestrator_.removeObserver(*this);
}

void CalibrationSessionControlViewModel::setCalibrationMode(domain::common::CalibrationMode mode) {
    selected_mode.set(mode);
}

void CalibrationSessionControlViewModel::startCalibration() {
    const auto settings = settings_query_.execute();

    application::orchestrators::CalibrationOrchestratorInput input{
        selected_mode.get_copy(),
        settings.pressure_unit,
        settings.angle_unit,
        settings.pressure_points
    };

    if (!orchestrator_.start(input)) {
        error_text.set("Не удалось запустить калибровку");
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
    std::visit(
        [this](const auto& e) {
            using T = std::decay_t<decltype(e)>;

            if constexpr (std::is_same_v<T, application::orchestrators::CalibrationOrchestratorEvent::Started>) {
                error_text.set("");
                syncState(application::orchestrators::CalibrationOrchestratorState::Started);
            } else if constexpr (std::is_same_v<T, application::orchestrators::CalibrationOrchestratorEvent::Stopped>) {
                syncState(application::orchestrators::CalibrationOrchestratorState::Stopped);
            } else if constexpr (std::is_same_v<T, application::orchestrators::CalibrationOrchestratorEvent::Failed>) {
                error_text.set(e.error);
                syncState(application::orchestrators::CalibrationOrchestratorState::Stopped);
            }
        },
        ev.data
    );
}

void CalibrationSessionControlViewModel::syncState(application::orchestrators::CalibrationOrchestratorState state) {
    const bool canStart = state == application::orchestrators::CalibrationOrchestratorState::Stopped;
    const bool canStop = state == application::orchestrators::CalibrationOrchestratorState::Starting
                         || state == application::orchestrators::CalibrationOrchestratorState::Started
                         || state == application::orchestrators::CalibrationOrchestratorState::Stopping;

    can_start.set(canStart);
    can_stop.set(canStop);
    can_abort.set(canStop);
}
