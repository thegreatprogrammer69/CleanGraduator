#include "CalibrationOrchestrator.h"

#include <vector>

#include "application/ports/video/IVideoAngleSourcesStorage.h"
#include "domain/core/angle/AngleSourcePacket.h"
#include "domain/core/calibration/strategy/CalibrationBeginContext.h"
#include "domain/core/calibration/strategy/CalibrationFeedContext.h"
#include "domain/ports/angle/IAngleSource.h"
#include "domain/ports/calibration/recording/ICalibrationRecorder.h"
#include "domain/ports/calibration/lifecycle/ICalibrationLifecycle.h"
#include "domain/ports/calibration/strategy/ICalibrationStrategy.h"
#include "domain/ports/drivers/motor/IMotorDriver.h"
#include "domain/ports/pressure/IPressureSource.h"

using namespace application::orchestrators;
using namespace domain::common;

CalibrationOrchestrator::CalibrationOrchestrator(CalibrationOrchestratorPorts ports)
    : pressure_points_tracker_(*this)
    , logger_(ports.logger)
    , pressure_source_(ports.pressure_source)
    , angle_sources_storage_(ports.angle_sources_storage)
    , motor_driver_(ports.motor_driver)
    , valve_driver_(ports.valve_driver)
    , strategy_(ports.strategy)
    , recorder_(ports.recorder)
    , lifecycle_(ports.lifecycle)
{
    // Orchestrator does NOT control motor/valve.
    // Strategy owns actuator commands via bind().
    strategy_.bind(motor_driver_, valve_driver_, recorder_);

    // Observe-only initial snapshots (read-only is ok)
    last_limits_.store(motor_driver_.limits());
    last_direction_.store(motor_driver_.direction());
}

CalibrationOrchestrator::~CalibrationOrchestrator() {
    // deterministic shutdown: emergency path (idempotent)
    abort();
}

CalibrationOrchestrator::LifecycleState CalibrationOrchestrator::lifecycleState() const {
    return lifecycle_.state();
}

bool CalibrationOrchestrator::isRunning() const {
    return lifecycleState() == LifecycleState::Running;
}

// -----------------------------------------------------
// Observers attach/detach (idempotent, split groups)
// -----------------------------------------------------
void CalibrationOrchestrator::attachSourceObservers() {
    if (sources_attached_.exchange(true))
        return;

    pressure_source_.addObserver(*this);

    for (auto& src : angle_sources_storage_.all()) {
        if (src.angle_source.isRunning()) {
            src.angle_source.addObserver(*this);
        }
    }

    logger_.info("Source observers attached");
}

void CalibrationOrchestrator::detachSourceObservers() {
    if (!sources_attached_.exchange(false))
        return;

    pressure_source_.removeObserver(*this);

    for (auto& src : angle_sources_storage_.all()) {
        if (src.angle_source.isRunning()) {
            src.angle_source.removeObserver(*this);
        }
    }

    logger_.info("Source observers detached");
}

void CalibrationOrchestrator::attachActuatorObservers() {
    if (actuators_attached_.exchange(true))
        return;

    motor_driver_.addObserver(*this);
    valve_driver_.addObserver(*this);

    logger_.info("Actuator observers attached");
}

void CalibrationOrchestrator::detachActuatorObservers() {
    if (!actuators_attached_.exchange(false))
        return;

    motor_driver_.removeObserver(*this);
    valve_driver_.removeObserver(*this);

    logger_.info("Actuator observers detached");
}

// --------------------------
// Session begin/end (idempotent)
// --------------------------
void CalibrationOrchestrator::beginSession() {
    if (session_begun_.exchange(true))
        return;

    current_point_index_.store(-1);

    std::vector<float> pp;
    pp.reserve(config_.pressure_points.value.size());
    for (const auto& p : config_.pressure_points.value) {
        pp.push_back(p.to(config_.pressure_unit));
    }

    // Use last observed direction (no motor control)
    pressure_points_tracker_.beginTracking(pp, last_direction_.load());
    tracking_begun_.store(true);

    CalibrationBeginContext ctx;
    ctx.pressure_points = config_.pressure_points;
    ctx.pressure_unit = config_.pressure_unit;
    ctx.calibration_mode = config_.calibration_mode;

    // Strategy begins calibration; it may start motor later from feed()
    strategy_.begin(ctx);

    lifecycle_.markRunning();
    logger_.info("Calibration entered Running");
}

void CalibrationOrchestrator::endSessionIfBegun() {
    if (session_begun_.exchange(false)) {
        // Strategy must put hardware into safe state (including motor stop/abort) by contract.
        strategy_.end();
    }

    if (tracking_begun_.exchange(false)) {
        pressure_points_tracker_.endTracking();
    }
}

// ----------------------------------------
// Public API: start/stop/abort
// ----------------------------------------
void CalibrationOrchestrator::start(CalibrationOrchestratorInput input) {

    const auto st = lifecycleState();

    if (st != LifecycleState::Idle){
        logger_.warn("Start rejected: lifecycle state invalid ({})",
                     static_cast<int>(st));
        return;
    }

    const auto limits = motor_driver_.limits();
    last_limits_.store(limits);

    if (!limits.home){
        logger_.warn("Start rejected: HOME limit inactive. Use returnToHome().");
        return;
    }

    // Ensure pressure source is running before session start
    if (!pressure_source_.isRunning()) {
        logger_.info("Starting pressure source...");
        if (!pressure_source_.start()) {
            logger_.error("Calibration start failed: pressure_source_.start() returned false");
            lifecycle_.markError("pressure source start failed");
            return;
        }
    }


    if (!lifecycle_.start()) {
        logger_.warn("Start rejected by lifecycle");
        return;
    }

    config_ = input;
    logger_.info("Calibration start requested");

    // Attach first to avoid losing early events
    attachActuatorObservers();
    attachSourceObservers();

    // Refresh observed snapshots (read-only)
    last_limits_.store(motor_driver_.limits());
    last_direction_.store(motor_driver_.direction());
    motor_running_.store(motor_driver_.isRunning()); // read-only; remove if you want *zero* motor method calls

    // Begin session immediately: we must process pressure packets even before motor starts
    beginSession();
}

void CalibrationOrchestrator::startHoming() {
    const auto st = lifecycleState();

    if (st != LifecycleState::Idle)
    {
        logger_.warn("returnToHome rejected: lifecycle state invalid ({})",
                     static_cast<int>(st));
        return;
    }

    const auto limits = motor_driver_.limits();
    last_limits_.store(limits);

    // Уже дома
    if (limits.home) {
        logger_.info("Homing skipped: already at HOME");
        return;
    }

    attachActuatorObservers();

    if (!lifecycle_.startHoming()) {
        logger_.warn("returnToHome rejected by lifecycle");
        detachActuatorObservers();
        return;
    }

    homing_stop_requested_.store(false);

    strategy_.beginHoming();
    logger_.info("Homing started");
}

void CalibrationOrchestrator::stopHoming() {
}

void CalibrationOrchestrator::stop() {
    requestGracefulStop("stop() requested");
}

void CalibrationOrchestrator::abort() {
    abortNow("abort() requested");
}

// ----------------------------------------
// Internal stop/abort paths
// ----------------------------------------
void CalibrationOrchestrator::requestGracefulStop(const char* reason) {

    const auto st = lifecycleState();

    if (strlen(reason) > 0) {
        logger_.warn("Graceful stop with error: {}", reason);
        detachSourceObservers();
        detachActuatorObservers();
        lifecycle_.markIdle();
        return;
    }

    if (st == LifecycleState::Idle)
        return;

    if (st == LifecycleState::Starting) {
        cancelStartToIdle(reason);
        return;
    }

    if (st != LifecycleState::Running) {
        logger_.warn("Graceful stop ignored: lifecycle state not Running");
        return;
    }

    logger_.info("Calibration graceful stop requested: {}", reason);

    if (!lifecycle_.stop()) {
        logger_.warn("Lifecycle rejected stop()");
        return;
    }

    // Stop feeding strategy first
    detachSourceObservers();

    // End session (strategy.end() is where motor stop should happen)
    endSessionIfBegun();

    // If motor is not running, finalize immediately; otherwise wait for onMotorStopped()
    if (!motor_running_.load()) {
        detachActuatorObservers();
        lifecycle_.markIdle();
        logger_.info("Calibration stopped -> Idle (motor already stopped)");
    }
}

void CalibrationOrchestrator::cancelStartToIdle(const char* reason) {
    logger_.info("Cancel start -> Idle: {}", reason);

    detachSourceObservers();
    endSessionIfBegun();

    // No motor control here; strategy.end() is responsible for safe hardware state
    detachActuatorObservers();
    lifecycle_.markIdle();
}

void CalibrationOrchestrator::abortNow(const char* reason) {

    const auto st = lifecycleState();
    if (st == LifecycleState::Idle)
        return;

    logger_.error("Calibration abort: {}", reason);

    // Stop feeding immediately (prevents further strategy.feed)
    detachSourceObservers();

    // End session (strategy must perform emergency-safe actions by contract)
    endSessionIfBegun();

    // Mark lifecycle error
    lifecycle_.markError(reason);

    // Keep actuator observers until motor actually stops (for determinism/logging),
    // but if it's already stopped we can detach now.
    if (!motor_running_.load()) {
        detachActuatorObservers();
    }
}

// ----------------------------------------
// Pressure points tracker observer
// ----------------------------------------
void CalibrationOrchestrator::onPointEntered(int index) {
    current_point_index_.store(index);
    logger_.info("Entered pressure point {}", index);
}

void CalibrationOrchestrator::onPointExited(int index) {
    const auto cur = current_point_index_.load();
    if (cur == index) {
        current_point_index_.store(-1);
    }
    logger_.info("Exited pressure point {}", index);
}

// ----------------------------------------
// Motor callbacks (observe-only)
// ----------------------------------------
void CalibrationOrchestrator::onMotorStarted() {
    motor_running_.store(true);

    const auto st = lifecycleState();
    logger_.info("Motor started (lifecycle={})", static_cast<int>(st));
}

void CalibrationOrchestrator::onMotorStopped() {
    motor_running_.store(false);

    const auto st = lifecycleState();

    if (st == LifecycleState::Stopping) {
        logger_.info("Motor stopped: finalize -> Idle");
        detachActuatorObservers();
        lifecycle_.markIdle();
        return;
    }

    logger_.warn("onMotorStopped ignored in state {}", static_cast<int>(st));
}

void CalibrationOrchestrator::onMotorStartFailed(const MotorDriverError&) {
    abortNow("motor start failed callback");
}

void CalibrationOrchestrator::onMotorLimitsStateChanged(MotorLimitsState s) {
    last_limits_.store(s);
}

void CalibrationOrchestrator::onMotorDirectionChanged(MotorDirection d) {
    last_direction_.store(d);
}

// ----------------------------------------
// Valve callbacks (observe-only safety)
// ----------------------------------------
void CalibrationOrchestrator::onInputFlapOpened() {
    if (motor_running_.load()) {
        abortNow("input flap opened while motor running");
    }
}

void CalibrationOrchestrator::onOutputFlapOpened() {
    if (motor_running_.load()) {
        abortNow("output flap opened while motor running");
    }
}

void CalibrationOrchestrator::onFlapsClosed() {
    // ignore
}

// ----------------------------------------
// Pressure source callbacks
// ----------------------------------------
void CalibrationOrchestrator::onPressurePacket(const PressurePacket& p) {

    if (lifecycleState() != LifecycleState::Running)
        return;

    const float ts = p.timestamp.asSeconds();
    const float pressure = p.pressure.to(config_.pressure_unit);

    recorder_.pushPressure(ts, pressure);
    pressure_points_tracker_.update(pressure);

    domain::common::CalibrationFeedContext ctx;
    ctx.timestamp = ts;
    ctx.pressure = pressure;
    ctx.mode = config_.calibration_mode;
    ctx.limits = last_limits_.load();

    const int idx = current_point_index_.load();
    ctx.current_point = (idx >= 0) ? std::optional<int>(idx) : std::nullopt;

    const auto decision = strategy_.feed(ctx);

    if (decision == domain::ports::CalibrationDecisionType::Finish) {
        requestGracefulStop("strategy decision: Finish");
        return;
    }

    if (decision == domain::ports::CalibrationDecisionType::Fault) {
        abortNow("strategy decision: Fault");
        return;
    }
}

void CalibrationOrchestrator::onPressureSourceOpened() {
    logger_.info("Pressure source opened");
}

void CalibrationOrchestrator::onPressureSourceOpenFailed(const PressureSourceError&) {
    abortNow("pressure source open failed");
}

void CalibrationOrchestrator::onPressureSourceClosed(const PressureSourceError&) {
    const auto st = lifecycleState();
    if (st == LifecycleState::Starting || st == LifecycleState::Running) {
        abortNow("pressure source closed during calibration");
    }
}

// ----------------------------------------
// Angle source callbacks
// ----------------------------------------
void CalibrationOrchestrator::onAngleSourceStarted() {
    // ignore (if you need dynamic attach: attach on start here)
}

void CalibrationOrchestrator::onAngleSourceStopped() {
    const auto st = lifecycleState();
    if (st == LifecycleState::Starting || st == LifecycleState::Running) {
        abortNow("angle source stopped during calibration");
    }
}

void CalibrationOrchestrator::onAngleSourceFailed(const AngleSourceError&) {
    abortNow("angle source failed during calibration");
}

void CalibrationOrchestrator::onAngleSourcePacket(const AngleSourcePacket& p) {

    if (lifecycleState() != LifecycleState::Running)
        return;

    recorder_.pushAngle(
        p.source_id,
        p.timestamp.asSeconds(),
        p.angle.to(config_.angle_unit));
}