#include "AppStatusBarViewModel.h"

#include "application/orchestrators/calibration/process/CalibrationOrchestrator.h"
#include "domain/ports/clock/IClock.h"

mvvm::AppStatusBarViewModel::AppStatusBarViewModel(AppStatusBarViewModelDeps deps)
    : orchestrator_(deps.orchestrator)
    , session_clock_(deps.session_clock)
    , uptime_clock_(deps.uptime_clock)
{
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
    // current_state_.store({}, std::memory_order_relaxed);
}

