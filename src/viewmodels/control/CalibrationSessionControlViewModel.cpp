#include "CalibrationSessionControlViewModel.h"

using namespace mvvm;

CalibrationSessionControlViewModel::CalibrationSessionControlViewModel(CalibrationSessionControlViewModelDeps deps)
    : session_controller_(deps.session_controller)
    , lifecycle_(deps.lifecycle)
{
    lifecycle_.addObserver(*this);
    onCalibrationLifecycleStateChanged(lifecycle_.state(), lifecycle_.lastError());
}

CalibrationSessionControlViewModel::~CalibrationSessionControlViewModel() {
    lifecycle_.removeObserver(*this);
}

void CalibrationSessionControlViewModel::setCalibrationMode(domain::common::CalibrationMode mode) {
    selected_mode.set(mode);
}

void CalibrationSessionControlViewModel::startCalibration() {
    const application::orchestrators::CalibrationSessionControllerInput input{selected_mode.get_copy()};
    session_controller_.start(input);
}

void CalibrationSessionControlViewModel::stopCalibration() {
    session_controller_.stop();
}

void CalibrationSessionControlViewModel::emergencyStop() {
    session_controller_.abort();
}

void CalibrationSessionControlViewModel::onCalibrationLifecycleStateChanged(
    domain::common::CalibrationLifecycleState newState,
    const std::string& lastError)
{
    error_text.set(lastError);

    const bool isIdle = newState == domain::common::CalibrationLifecycleState::Idle;
    const bool isError = newState == domain::common::CalibrationLifecycleState::Error;
    const bool canStopNow =
        newState == domain::common::CalibrationLifecycleState::Starting
        || newState == domain::common::CalibrationLifecycleState::Running
        || newState == domain::common::CalibrationLifecycleState::Stopping;

    can_start.set(isIdle || isError);
    can_stop.set(canStopNow);
    can_abort.set(!isIdle);
}
