#include "Stand4CalibrationStrategy.h"

#include "domain/core/calibration/recording/CalibrationSessionId.h"
#include "domain/core/calibration/strategy/CalibrationStrategyBeginContext.h"
#include "domain/core/calibration/strategy/CalibrationStrategyFeedContext.h"
#include "domain/core/drivers/motor/MotorFlapsState.h"

#include "infrastructure/calibration/tracking/PressurePointsTrackerEvent.h"

#include <algorithm>
#include <iomanip>
#include <sstream>

using namespace domain;
using namespace domain::common;
using namespace domain::ports;

using namespace infra::calib;
using namespace infra::calib::stand4;
using namespace infra::calib::tracking;

namespace
{
constexpr int kMaxMotorFrequencyHz = 2000;

const char* toString(Stand4CalibrationStrategy::State s)
{
    switch (s) {
        case Stand4CalibrationStrategy::State::Idle: return "Idle";
        case Stand4CalibrationStrategy::State::Preload: return "Preload";
        case Stand4CalibrationStrategy::State::Forward: return "Forward";
        case Stand4CalibrationStrategy::State::Backward: return "Backward";
        case Stand4CalibrationStrategy::State::Finished: return "Finished";
        case Stand4CalibrationStrategy::State::Fault: return "Fault";
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
    , points_tracker_(*this)
{
    (void)config;
}

CalibrationStrategyVerdict
Stand4CalibrationStrategy::begin(const CalibrationStrategyBeginContext& ctx)
{
    Verdict v;

    logger_.info("Запуск стратегии калибровки Stand4");

    pressure_points_ = ctx.pressure_points.to(ctx.pressure_unit);
    calibration_mode_ = ctx.calibration_mode;

    if (calibration_mode_ == CalibrationMode::OnlyLast && pressure_points_.size() >= 2) {
        pressure_points_ = { pressure_points_.front(), pressure_points_.back() };
    }

    last_pressure_ = 0.0;
    last_time_ = 0.0;
    p_backward_start_ = 0.0;
    p_backward_target_ = 0.0;
    last_status_text_.clear();

    p_preload_  = computePreloadPressure(ctx.pressure_points).to(ctx.pressure_unit);
    p_target_   = computeTargetPressure(ctx.pressure_points).to(ctx.pressure_unit);
    p_limit_    = computeLimitPressure(ctx.pressure_points).to(ctx.pressure_unit);
    const double slowdown_multiplier = ctx.slowdown_at_checkpoints ? 0.7 : 1.0;
    dp_nominal_ = computeNominalVelocity(ctx.pressure_points).to(ctx.pressure_unit) * slowdown_multiplier;

    points_tracker_.setEnterThreshold(0.15);
    points_tracker_.setExitThreshold(0.1);

    logger_.info(
        "Параметры калибровки: preload={}, target={}, limit={}, dp_nominal={}",
        p_preload_,
        p_target_,
        p_limit_,
        dp_nominal_);

    transitionToPreload(v);

    return v;
}

CalibrationStrategyVerdict
Stand4CalibrationStrategy::feed(const CalibrationStrategyFeedContext& ctx)
{
    Verdict v;

    const State current = state_.load();

    if (current == State::Finished) {
        logger_.info("Получен feed в состоянии Finished");
        v.commands.push_back(Verdict::Complete{});
        return v;
    }

    if (current == State::Idle) {
        logger_.warn("Получен feed в состоянии Idle");
        return v;
    }

    if (current == State::Fault) {
        logger_.error("Получен feed в состоянии Fault");
        return v;
    }

    points_tracker_.feed(ctx.pressure);

    switch (current) {
        case State::Preload:   updatePreload(ctx, v); break;
        case State::Forward:   updateForward(ctx, v); break;
        case State::Backward:  updateBackward(ctx, v); break;
        default: break;
    }

    last_pressure_ = ctx.pressure;
    last_time_ = ctx.timestamp;

    v.commands.insert(
        v.commands.end(),
        pending_.begin(),
        pending_.end());

    pending_.clear();

    return v;
}

CalibrationStrategyVerdict
Stand4CalibrationStrategy::end()
{
    Verdict v;

    logger_.info("Принудительное завершение стратегии");

    v.commands.push_back(
        Verdict::MotorSetFlaps{MotorFlapsState::FlapsClosed});

    v.commands.push_back(Verdict::MotorStop{});
    v.commands.push_back(Verdict::EndSession{});

    points_tracker_.endTracking();

    transition(State::Idle);

    return v;
}

bool Stand4CalibrationStrategy::isRunning() const
{
    const auto s = state_.load();

    return s != State::Idle &&
           s != State::Finished &&
           s != State::Fault;
}

void Stand4CalibrationStrategy::onPressurePointsTrackerEvent(
    const PressurePointsTrackerEvent& ev)
{
    const auto s = state_.load();

    if (s == State::Idle || s == State::Finished || s == State::Fault)
        return;

    std::visit([this, s](const auto& e)
    {
        using T = std::decay_t<decltype(e)>;

        if constexpr (std::is_same_v<T, PressurePointsTrackerEvent::PointEntered>)
        {
            MotorDirection direction;

            if (s == State::Preload || s == State::Forward)
                direction = MotorDirection::Forward;
            else if (s == State::Backward)
                direction = MotorDirection::Backward;
            else
                return;

            logger_.info(
                "Вход в зону точки {} по направлению {}",
                e.index.id,
                direction);

            CalibrationSessionId session;
            session.point = e.index;
            session.direction = direction;

            pending_.push_back(Verdict::Command{Verdict::BeginSession{session}});
        }

        else if constexpr (std::is_same_v<T, PressurePointsTrackerEvent::PointExited>)
        {
            pending_.emplace_back(Verdict::EndSession{});
        }

    }, ev.data);
}

/* ============================
   UPDATE METHODS
   ============================ */

void Stand4CalibrationStrategy::updatePreload(
    const CalibrationStrategyFeedContext& ctx,
    Verdict& v)
{
    const float p_cur = ctx.pressure;

    logger_.info(
        "Preload: текущее давление={}, требуемое давление={}",
        p_cur,
        p_preload_);

    emitStatus(v, buildPreloadStatusText(p_cur));

    if (p_cur < p_preload_) {
        v.commands.push_back(Verdict::MotorSetFlaps{MotorFlapsState::IntakeOpened});
        return;
    }

    logger_.info("Достигнуто давление преднагрузки {}", p_cur);

    transitionToForward(v);
}

void Stand4CalibrationStrategy::updateForward(
    const CalibrationStrategyFeedContext& ctx,
    Verdict& v)
{
    const float p_cur = ctx.pressure;
    const float dt = ctx.timestamp - last_time_;

    const float dp_cur =
        (last_time_ > 0.0f && dt > 0.0f)
        ? (p_cur - last_pressure_) / dt
        : 0.0f;

    logger_.info(
        "Forward: давление={}, скорость={}, dt={}",
        p_cur,
        dp_cur,
        dt);

    if (ctx.limits_state.end) {
        logger_.info("Достигнут конечный концевик на прямом ходе");
        transitionToBackward(v);
        return;
    }

    if (p_cur >= p_target_) {
        logger_.info(
            "Достигнуто целевое давление {} >= {}",
            p_cur,
            p_target_);
        transitionToBackward(v);
        return;
    }

    const int freq = freq_calc_.frequency(
        p_cur,
        p_target_,
        dp_cur,
        dp_nominal_);
    const int mode_freq = calibration_mode_ == CalibrationMode::OnlyLast
        ? std::min(freq * 2, kMaxMotorFrequencyHz)
        : freq;

    logger_.info("Расчёт частоты двигателя {}", mode_freq);

    v.commands.push_back(
        Verdict::MotorSetFrequency{mode_freq});
}

void Stand4CalibrationStrategy::updateBackward(const CalibrationStrategyFeedContext& ctx, Verdict& v)
{
    // if (ctx.pressure < 15) {
    //     logger_.info("Двигатель дошёл до конечного концевика");
    //     transitionToFinished(v);
    //     return;
    // }

    if (ctx.limits_state.home) {
        logger_.info("Двигатель дошёл до конечного концевика");
        transitionToFinished(v);
        return;
    }
    /*
     *Когда программа только запущена, и камеры не открывались, в комбо-боксе выбрана 1 камера, но в строке камер тогда надо тоже написать 1. А ещё, при выборе в комбо боксе кол-ва камер, нужн автоматич
     **/

    const float p_cur = ctx.pressure;
    const float dt = ctx.timestamp - last_time_;

    const float dp_cur =
        (last_time_ > 0.0f && dt > 0.0f)
        ? (p_cur - last_pressure_) / dt
        : 0.0f;

    int frequency = kMaxMotorFrequencyHz;

    if (calibration_mode_ == CalibrationMode::Full) {
        const double backward_progress = std::max(0.0, p_backward_start_ - static_cast<double>(p_cur));
        const double backward_target = std::max(0.001, p_backward_start_ - p_backward_target_);
        frequency = freq_calc_.frequency(
            backward_progress,
            backward_target,
            -dp_cur,
            dp_nominal_ / 1.15);
    }

    logger_.info("Возврат двигателя домой частота {}", frequency);

    v.commands.push_back(
        Verdict::MotorSetFrequency{frequency});
}

/* ============================
   TRANSITIONS
   ============================ */

void Stand4CalibrationStrategy::transition(State newState)
{
    const State old = state_.load();

    logger_.info(
        "Переход состояния {} -> {}",
        toString(old),
        toString(newState));

    state_ = newState;
}

void Stand4CalibrationStrategy::transitionToPreload(Verdict& v)
{
    transition(State::Preload);

    v.commands.push_back(
        Verdict::MotorSetFlaps{MotorFlapsState::FlapsClosed});

    points_tracker_.beginTracking(
        pressure_points_,
        MotorDirection::Forward);

    emitStatus(v, buildPreloadStatusText(static_cast<float>(last_pressure_)));
}

void Stand4CalibrationStrategy::transitionToForward(Verdict& v)
{
    transition(State::Forward);

    freq_calc_.reset();

    v.commands.push_back(
        Verdict::MotorSetFlaps{MotorFlapsState::FlapsClosed});

    v.commands.push_back(
        Verdict::MotorSetDirection{MotorDirection::Forward});

    v.commands.push_back(
        Verdict::MotorSetFrequency{0});

    v.commands.push_back(
        Verdict::MotorStart{});

    emitStatus(v, "Прямой ход: съём показаний");
}

void Stand4CalibrationStrategy::transitionToBackward(Verdict& v)
{
    transition(State::Backward);
    freq_calc_.reset();
    p_backward_start_ = last_pressure_;
    p_backward_target_ = p_preload_;

    v.commands.push_back(Verdict::EndSession{});

    points_tracker_.endTracking();

    points_tracker_.beginTracking(
        pressure_points_,
        MotorDirection::Backward);

    v.commands.push_back(
        Verdict::MotorSetFrequency{0});

    v.commands.push_back(
        Verdict::MotorSetDirection{MotorDirection::Backward});

    emitStatus(v, "Обратный ход: возврат и съём");
}

void Stand4CalibrationStrategy::transitionToFinished(Verdict& v)
{
    transition(State::Finished);

    v.commands.push_back(Verdict::EndSession{});

    points_tracker_.endTracking();

    v.commands.push_back(Verdict::MotorStop{});

    v.commands.push_back(Verdict::MotorSetFlaps{MotorFlapsState::ExhaustOpened});

    emitStatus(v, "Градуировка завершена");
    v.commands.push_back(Verdict::Complete{});
}

void Stand4CalibrationStrategy::transitionToFault(Verdict& v)
{
    transition(State::Fault);

    v.commands.push_back(Verdict::EndSession{});
    v.commands.push_back(Verdict::MotorStop{});

    points_tracker_.endTracking();
    emitStatus(v, "Аварийная остановка: ошибка стратегии");
}

void Stand4CalibrationStrategy::emitStatus(Verdict& v, std::string text)
{
    if (text.empty() || text == last_status_text_) {
        return;
    }

    last_status_text_ = text;
    v.commands.push_back(Verdict::StatusText{std::move(text)});
}

std::string Stand4CalibrationStrategy::buildPreloadStatusText(float current_pressure) const
{
    const double safe_target = std::max(0.001, p_preload_);
    const double progress = std::clamp(static_cast<double>(current_pressure) / safe_target, 0.0, 1.0);
    const int progress_percent = static_cast<int>(progress * 100.0 + 0.5);

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(1)
       << "Набор предварительного давления: "
       << current_pressure
       << " / "
       << p_preload_
       << " ("
       << progress_percent
       << "%)";

    return ss.str();
}
