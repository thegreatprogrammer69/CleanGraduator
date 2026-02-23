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

bool CalibrationSessionControlViewModel::start() {
    return lifecycle_.start();
}

void CalibrationSessionControlViewModel::markRunning() {
    lifecycle_.markRunning();
}

bool CalibrationSessionControlViewModel::stop() {
    return lifecycle_.stop();
}

void CalibrationSessionControlViewModel::markIdle() {
    lifecycle_.markIdle();
}

void CalibrationSessionControlViewModel::markError(const std::string& err) {
    lifecycle_.markError(err);
}

std::string CalibrationSessionControlViewModel::lastError() const {
    return lifecycle_.lastError();
}

domain::common::CalibrationLifecycleState CalibrationSessionControlViewModel::state() const {
    return lifecycle_.state();
}

void CalibrationSessionControlViewModel::addObserver(domain::ports::ICalibrationLifecycleObserver& observer) {
    lifecycle_.addObserver(observer);
}

void CalibrationSessionControlViewModel::removeObserver(domain::ports::ICalibrationLifecycleObserver& observer) {
    lifecycle_.removeObserver(observer);
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
