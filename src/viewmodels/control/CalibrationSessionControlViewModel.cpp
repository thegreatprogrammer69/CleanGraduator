#include "CalibrationSessionControlViewModel.h"

using namespace mvvm;
using namespace application::orchestrators;

CalibrationSessionControlViewModel::CalibrationSessionControlViewModel(CalibrationSessionControlViewModelDeps deps)
    : controller_(deps.controller)
{
    controller_.addObserver(*this);
}

CalibrationSessionControlViewModel::~CalibrationSessionControlViewModel()
{
    controller_.removeObserver(*this);
}

void CalibrationSessionControlViewModel::setCalibrationMode(domain::common::CalibrationMode mode)
{
    calibration_mode_ = mode;
}

domain::common::CalibrationMode CalibrationSessionControlViewModel::calibrationMode() const
{
    return calibration_mode_;
}

void CalibrationSessionControlViewModel::start()
{
    CalibrationSessionControllerInput input{};
    input.calibration_mode = calibration_mode_;
    controller_.start(input);
}

void CalibrationSessionControlViewModel::stop()
{
    controller_.stop();
}

void CalibrationSessionControlViewModel::emergencyStop()
{
    controller_.abort();
}

void CalibrationSessionControlViewModel::onCalibrationLifecycleStateChanged(
    domain::common::CalibrationLifecycleState new_state,
    const std::string& last_error)
{
    using S = domain::common::CalibrationLifecycleState;

    const bool running =
        new_state == S::Starting ||
        new_state == S::Running ||
        new_state == S::Stopping;

    is_running.set(running);
    error_text.set(new_state == S::Error ? last_error : std::string{});
}
