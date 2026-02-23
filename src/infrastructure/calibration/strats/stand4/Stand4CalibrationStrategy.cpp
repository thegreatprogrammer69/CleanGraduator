#include "Stand4CalibrationStrategy.h"

#include "domain/core/calibration/strategy/CalibrationBeginContext.h"
#include "domain/core/calibration/strategy/CalibrationFeedContext.h"

using namespace domain;
using namespace domain::common;
using namespace domain::ports;

using namespace infra::calib::stand4;

Stand4CalibrationStrategy::Stand4CalibrationStrategy()
    : freq_calc_(
        0.1, 10.0,
        10.0, 2000.0,
        1.7, 0.7,
        1.0)
{
}

void Stand4CalibrationStrategy::bind(IMotorDriver& motor, IValveDriver& valve, ICalibrationRecorder& recorder)
{
    motor_ = &motor;
    valve_ = &valve;
    recorder_ = &recorder;
}

void Stand4CalibrationStrategy::begin(const CalibrationBeginContext& ctx)
{
    state_ = State::Preload;

    last_pressure_ = 0.0;
    last_time_ = 0.0;

    p_preload_ = computePreloadPressure(ctx.pressure_points).to(ctx.pressure_unit);
    p_target_ = computeTargetPressure(ctx.pressure_points).to(ctx.pressure_unit);
    p_limit_ = computeLimitPressure(ctx.pressure_points).to(ctx.pressure_unit);
    dp_nominal_ = computeNominalVelocity(ctx.pressure_points).to(ctx.pressure_unit);

    freq_calc_.reset();

    valve_->closeFlaps();
}

void Stand4CalibrationStrategy::end()
{
    valve_->closeFlaps();
    motor_->stop();
    state_ = State::Idle;
}

bool Stand4CalibrationStrategy::isRunning() const
{
    const auto s = state_.load();
    return s != State::Idle &&
           s != State::Finished &&
           s != State::Fault;
}

CalibrationDecisionType Stand4CalibrationStrategy::feed(const CalibrationFeedContext& ctx)
{
    const State current = state_.load();

    if (current == State::Idle ||
        current == State::Finished ||
        current == State::Fault)
        return CalibrationDecisionType::None;

    switch (current) {
        case State::Preload:
            updatePreload(ctx);
            break;

        case State::Forward:
            updateForward(ctx);
            break;

        case State::Backward:
            updateBackward(ctx);
            break;

        default:
            break;
    }

    last_pressure_ = ctx.pressure;;
    last_time_ = ctx.timestamp;

    return CalibrationDecisionType::None;
}

void Stand4CalibrationStrategy::updatePreload(const CalibrationFeedContext& ctx)
{
    const float p_cur = ctx.pressure;

    if (p_cur < p_preload_) {
        valve_->openInputFlap();
        return;
    }

    valve_->closeFlaps();
    state_ = State::Forward;

    motor_->setDirection(MotorDirection::Forward);
    motor_->setFrequency(0);
    motor_->start();
}

void Stand4CalibrationStrategy::updateForward(const CalibrationFeedContext& ctx)
{
    const float p_cur = ctx.pressure;
    const float dt = ctx.timestamp - last_time_;
    const float dp_cur =
        (last_time_ > 0.0 && dt > 0.0)
            ? (p_cur - last_pressure_) / dt
            : 0.0;

    if (p_cur >= p_target_) {
        motor_->setFrequency(0);
        motor_->setDirection(MotorDirection::Backward);
        state_ = State::Backward;
        return;
    }

    const int freq = freq_calc_.frequency(
        p_cur,
        p_target_,
        dp_cur,
        dp_nominal_);

    motor_->setFrequency(freq);
}

void Stand4CalibrationStrategy::updateBackward(const CalibrationFeedContext& ctx)
{
    if (ctx.limits.home) {
        state_ = State::Finished;
        motor_->stop();
        return;
    }

    const int f_max = motor_->frequencyLimits().maxHz;

    motor_->setFrequency(f_max);
}
