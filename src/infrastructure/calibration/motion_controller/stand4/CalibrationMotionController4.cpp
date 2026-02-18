#include "CalibrationMotionController4.h"

#include "domain/ports/Motor/IValveDriver.h"
#include "domain/ports/Motor/IMotorDriver.h"

infra::calib::CalibrationMotionController4::CalibrationMotionController4(
    CalibrationMotionControllerPorts ports, CalibrationMotionController4Config config)
        : logger_(ports.logger)
        , motor_driver_(ports.motor_driver)
        , valve_driver_(ports.dual_valve_driver)
        , config_(config)
        , points_tracker_(*this)
{
    motor_driver_.addObserver(*this);
}

infra::calib::CalibrationMotionController4::~CalibrationMotionController4() {
    motor_driver_.removeObserver(*this);
    valve_driver_.removeObserver(*this);
}

void infra::calib::CalibrationMotionController4::start(domain::ports::CalibrationMotionInput input) {
    if (is_started_.load(std::memory_order_acquire)) {
        logger_.warn("start(): Controller already started.");
        return;
    }

    if (is_motor_started_.load(std::memory_order_acquire)) {
        logger_.warn("start(): Motor already running externally. Start rejected.");
        return;
    }

    is_started_ = true;
    valve_driver_.addObserver(*this);
}

void infra::calib::CalibrationMotionController4::stop() {
    is_started_ = false;
    motor_driver_.stop();
    // return to home
}

void infra::calib::CalibrationMotionController4::abort() {
    is_started_ = false;
    motor_driver_.stop();
}

bool infra::calib::CalibrationMotionController4::isRunning() const {
    return is_started_.load(std::memory_order_relaxed) && is_motor_started_.load(std::memory_order_relaxed);
}


void infra::calib::CalibrationMotionController4::onStarted() {
    is_motor_started_.store(true, std::memory_order_relaxed);
    if (is_started_.load(std::memory_order_relaxed)) {
        logger_.info("The Motor and Controller have been started successfully.");
    }
    else {
        logger_.info("The Motor have been started by another process, ignoring it.");
    }
}

void infra::calib::CalibrationMotionController4::onStopped() {
    if (isRunning()) {
        logger_.error("The Motor was stopped while the Controller was running. Shotdowning Controller...");
    }
    is_motor_started_.store(false, std::memory_order_relaxed);
    abort();
}

void infra::calib::CalibrationMotionController4::onLimitsStateChanged(domain::common::MotorLimitsState state) {
    if (!isRunning())
        return;

    if (state.end) {
        logger_.error(
            "End limit switch activated during calibration. "
            "Stopping controller."
        );
        abort();
    }
}

void infra::calib::CalibrationMotionController4::onDirectionChanged(domain::common::MotorDirection dir) {
    if (!isRunning())
        return;

    const auto expected = current_direction_.load(std::memory_order_relaxed);

    if (expected != dir) {
        logger_.error(
            "MotorDirection invariant violated. "
            "Expected={}, actual={}. Controller will stop.",
            expected, dir
        );
        abort();
        return;
    }

    logger_.info("MotorDirection confirmed: {}", dir);
}

void infra::calib::CalibrationMotionController4::onFault(const domain::common::MotorFault &fault) {
    if (isRunning()) {
        logger_.error("The Motor was failed while the Controller was running. Shotdowning Controller...");
    }
    is_motor_started_.store(false, std::memory_order_relaxed);
    abort();
}

void infra::calib::CalibrationMotionController4::onInputFlapOpened() {
    if (isRunning()) {
        logger_.error("The input flap was opened during Motor control. "
                     "This could have affected. Shotdowning Controller and Motor...");
    }
}

void infra::calib::CalibrationMotionController4::onOutputFlapOpened() {
    if (isRunning()) {
        logger_.error("The output flap was opened during Motor control. "
                     "This could have affected. Shotdowning Controller and Motor...");
    }
}

void infra::calib::CalibrationMotionController4::onFlapsClosed() {
    if (isRunning()) {
        logger_.warn("The valves were closed during Motor control. It doesn't affect, continuing...");
    }
}

void infra::calib::CalibrationMotionController4::onPointEntered(int index) {

}

void infra::calib::CalibrationMotionController4::onPointExited(int index) {
}

void infra::calib::CalibrationMotionController4::addObserver(domain::ports::ICalibrationMotionObserver &) {
}

void infra::calib::CalibrationMotionController4::removeObserver(domain::ports::ICalibrationMotionObserver &) {
}
