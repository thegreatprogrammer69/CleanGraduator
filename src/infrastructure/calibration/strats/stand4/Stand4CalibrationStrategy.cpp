#include "Stand4CalibrationStrategy.h"

#include "domain/core/calibration/strategy/CalibrationStrategyBeginContext.h"
#include "domain/core/calibration/strategy/CalibrationStrategyFeedContext.h"

using namespace domain;
using namespace domain::common;
using namespace domain::ports;

using namespace infra::calib::stand4;

namespace
{
    const char* toString(Stand4CalibrationStrategy::State s)
    {
        switch (s) {
            case Stand4CalibrationStrategy::State::Idle:      return "Idle";
            case Stand4CalibrationStrategy::State::Preload:   return "Preload";
            case Stand4CalibrationStrategy::State::Forward:   return "Forward";
            case Stand4CalibrationStrategy::State::Backward:  return "Backward";
            case Stand4CalibrationStrategy::State::Finished:  return "Finished";
            case Stand4CalibrationStrategy::State::Fault:     return "Fault";
            default: return "Unknown";
        }
    }
}

Stand4CalibrationStrategy::Stand4CalibrationStrategy(
    CalibrationStrategyPorts ports,
    Stand4CalibrationStrategyConfig config)
    : logger_(ports.logger)
    , freq_calc_(
        0.1, 10.0,
        10.0, 2000.0,
        1.7, 0.7,
        1.0)
{
    (void)config;
}

void Stand4CalibrationStrategy::bind(IMotorDriver& motor,
                                     ICalibrationRecorder& recorder)
{
    motor_ = &motor;
    recorder_ = &recorder;
}

CalibrationStrategyVerdict
Stand4CalibrationStrategy::begin(const CalibrationStrategyBeginContext& ctx)
{
    logger_.info("Запуск стратегии калибровки Stand4");

    if (!motor_ || !recorder_) {
        logger_.error("Стратегия не привязана: motor_={}, recorder_={}",
                      static_cast<bool>(motor_),
                      static_cast<bool>(recorder_));
        transitionToFault();
        return CalibrationStrategyVerdict(CalibrationStrategyVerdict::None());
    }

    last_pressure_ = 0.0;
    last_time_ = 0.0;

    p_preload_  = computePreloadPressure(ctx.pressure_points).to(ctx.pressure_unit);
    p_target_   = computeTargetPressure(ctx.pressure_points).to(ctx.pressure_unit);
    p_limit_    = computeLimitPressure(ctx.pressure_points).to(ctx.pressure_unit);
    dp_nominal_ = computeNominalVelocity(ctx.pressure_points).to(ctx.pressure_unit);

    logger_.info("Параметры калибровки: preload={}, target={}, limit={}, dp_nominal={}",
                 p_preload_, p_target_, p_limit_, dp_nominal_);

    freq_calc_.reset();

    transitionToPreload();

    return CalibrationStrategyVerdict(CalibrationStrategyVerdict::None());
}

void Stand4CalibrationStrategy::end()
{
    logger_.info("Принудительное завершение стратегии");

    if (motor_) {
        motor_->setFlapsState(MotorFlapsState::FlapsClosed);
        motor_->stop();
    }

    transition(State::Idle);
}

bool Stand4CalibrationStrategy::isRunning() const
{
    const auto s = state_.load();
    return s != State::Idle &&
           s != State::Finished &&
           s != State::Fault;
}

CalibrationStrategyVerdict
Stand4CalibrationStrategy::feed(const CalibrationStrategyFeedContext& ctx)
{
    const State current = state_.load();

    if (current == State::Finished) {
        logger_.info("Калибровка завершена");
        return CalibrationStrategyVerdict(CalibrationStrategyVerdict::Complete());
    }

    if (current == State::Idle) {
        logger_.warn("Получен feed в состоянии Idle");
        return CalibrationStrategyVerdict(CalibrationStrategyVerdict::None());
    }

    if (current == State::Fault) {
        logger_.error("Получен feed в состоянии Fault");
        return CalibrationStrategyVerdict(CalibrationStrategyVerdict::None());
    }

    switch (current) {
        case State::Preload:   updatePreload(ctx);  break;
        case State::Forward:   updateForward(ctx);  break;
        case State::Backward:  updateBackward(ctx); break;
        default: break;
    }

    last_pressure_ = ctx.pressure;
    last_time_ = ctx.timestamp;

    return CalibrationStrategyVerdict(CalibrationStrategyVerdict::None());
}

/* ============================
   UPDATE METHODS
   ============================ */

void Stand4CalibrationStrategy::updatePreload(
    const CalibrationStrategyFeedContext& ctx)
{
    const float p_cur = ctx.pressure;

    logger_.info("Preload: текущее давление={}, требуемое давление={}",
                 p_cur, p_preload_);

    if (p_cur < p_preload_) {
        motor_->setFlapsState(MotorFlapsState::IntakeOpened);
        return;
    }

    logger_.info("Достигнуто давление preload: {}", p_cur);

    transitionToForward();
}

void Stand4CalibrationStrategy::updateForward(
    const CalibrationStrategyFeedContext& ctx)
{
    const float p_cur = ctx.pressure;
    const float dt = ctx.timestamp - last_time_;

    if (dt <= 0.0f && last_time_ > 0.0f) {
        logger_.warn("Некорректный интервал времени: dt={}, last_time={}, timestamp={}",
                     dt, last_time_, ctx.timestamp);
    }

    const float dp_cur =
        (last_time_ > 0.0f && dt > 0.0f)
            ? (p_cur - last_pressure_) / dt
            : 0.0f;

    logger_.info("Forward: давление={}, скорость изменения={}, dt={}",
                 p_cur, dp_cur, dt);

    if (p_cur >= p_target_) {
        logger_.info("Достигнуто целевое давление: {} >= {}",
                     p_cur, p_target_);
        transitionToBackward();
        return;
    }

    const int freq = freq_calc_.frequency(
        p_cur,
        p_target_,
        dp_cur,
        dp_nominal_);

    logger_.info("Расчёт частоты двигателя: {}", freq);

    motor_->setFrequency(MotorFrequency(freq));
}

void Stand4CalibrationStrategy::updateBackward(
    const CalibrationStrategyFeedContext&)
{
    if (motor_->limits().home) {
        logger_.info("Достигнут начальный концевик, завершение калибровки");
        transitionToFinished();
        return;
    }

    const int f_max = motor_->frequencyLimits().maxHz;

    logger_.info("Возвращение двигателя домой, частота={}", f_max);

    motor_->setFrequency(MotorFrequency(f_max));
}

/* ============================
   TRANSITIONS
   ============================ */

void Stand4CalibrationStrategy::transition(State newState)
{
    const State old = state_.load();

    logger_.info("Переход состояния: {} -> {}",
                 toString(old), toString(newState));

    state_ = newState;
}

void Stand4CalibrationStrategy::transitionToPreload()
{
    transition(State::Preload);

    motor_->setFlapsState(MotorFlapsState::FlapsClosed);
}

void Stand4CalibrationStrategy::transitionToForward()
{
    transition(State::Forward);

    motor_->setFlapsState(MotorFlapsState::FlapsClosed);
    motor_->setDirection(MotorDirection::Forward);
    motor_->setFrequency(MotorFrequency(0));
    motor_->start();
}

void Stand4CalibrationStrategy::transitionToBackward()
{
    transition(State::Backward);

    motor_->setFrequency(MotorFrequency(0));
    motor_->setDirection(MotorDirection::Backward);
}

void Stand4CalibrationStrategy::transitionToFinished()
{
    transition(State::Finished);

    motor_->setFlapsState(MotorFlapsState::ExhaustOpened);
    motor_->stop();
}

void Stand4CalibrationStrategy::transitionToFault()
{
    transition(State::Fault);

    if (motor_) {
        motor_->stop();
    }
}