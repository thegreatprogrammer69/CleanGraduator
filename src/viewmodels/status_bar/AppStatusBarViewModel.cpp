#include "AppStatusBarViewModel.h"

#include "application/orchestrators/calibration/process/CalibrationOrchestrator.h"
#include "application/orchestrators/calibration/process/CalibrationOrchestratorEvent.h"
#include "domain/ports/clock/IClock.h"

#include <type_traits>
#include <variant>

mvvm::AppStatusBarViewModel::AppStatusBarViewModel(AppStatusBarViewModelDeps deps)
    : orchestrator_(deps.orchestrator)
    , session_clock_(deps.session_clock)
    , uptime_clock_(deps.uptime_clock)
{
    current_state_.store(
        orchestrator_.isRunning()
            ? application::orchestrators::CalibrationOrchestratorState::Started
            : application::orchestrators::CalibrationOrchestratorState::Stopped,
        std::memory_order_relaxed);
    orchestrator_.addObserver(*this);
}

mvvm::AppStatusBarViewModel::~AppStatusBarViewModel() {
    orchestrator_.removeObserver(*this);
}

application::orchestrators::CalibrationOrchestratorState mvvm::AppStatusBarViewModel::state() {
    return current_state_.load(std::memory_order_relaxed);
}

domain::common::Timestamp mvvm::AppStatusBarViewModel::sessionTime() {
    return session_clock_.now();
}

domain::common::Timestamp mvvm::AppStatusBarViewModel::uptimeTime() {
    return uptime_clock_.now();
}

void mvvm::AppStatusBarViewModel::onCalibrationOrchestratorEvent(
    const application::orchestrators::CalibrationOrchestratorEvent &ev) {
    std::visit([this](const auto& event) {
        using T = std::decay_t<decltype(event)>;

        if constexpr (std::is_same_v<T, application::orchestrators::CalibrationOrchestratorEvent::Started>) {
            current_state_.store(application::orchestrators::CalibrationOrchestratorState::Started, std::memory_order_relaxed);
        } else if constexpr (std::is_same_v<T, application::orchestrators::CalibrationOrchestratorEvent::Stopped>
                             || std::is_same_v<T, application::orchestrators::CalibrationOrchestratorEvent::Failed>) {
            current_state_.store(application::orchestrators::CalibrationOrchestratorState::Stopped, std::memory_order_relaxed);
        }
    }, ev.data);
}
