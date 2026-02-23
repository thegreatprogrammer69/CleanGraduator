#include "AppStatusBarViewModel.h"
#include "domain/ports/clock/IClock.h"
#include "domain/ports/calibration/lifecycle/ICalibrationLifecycle.h"

mvvm::AppStatusBarViewModel::AppStatusBarViewModel(AppStatusBarViewModelDeps deps)
    : lifecycle_(deps.lifecycle)
    , session_clock_(deps.session_clock)
    , uptime_clock_(deps.uptime_clock)
{
    lifecycle_.addObserver(*this);
}

mvvm::AppStatusBarViewModel::~AppStatusBarViewModel() {
    lifecycle_.removeObserver(*this);
}

domain::common::CalibrationLifecycleState mvvm::AppStatusBarViewModel::state() {
    return current_state_.load(std::memory_order_relaxed);
}

domain::common::Timestamp mvvm::AppStatusBarViewModel::sessionTime() {
    return session_clock_.now();
}

domain::common::Timestamp mvvm::AppStatusBarViewModel::uptimeTime() {
    return uptime_clock_.now();
}

void mvvm::AppStatusBarViewModel::onCalibrationLifecycleStateChanged(domain::common::CalibrationLifecycleState new_state, const std::string& lastError) {
    current_state_.store(new_state, std::memory_order_relaxed);
}
