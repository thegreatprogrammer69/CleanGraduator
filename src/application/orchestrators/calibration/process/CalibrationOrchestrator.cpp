#include "CalibrationOrchestrator.h"

#include <algorithm>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <utility>
#include <variant>

#include "application/orchestrators/video/VideoSourceManager.h"
#include "application/ports/video/IVideoAngleSourcesStorage.h"
#include "domain/core/angle/AngleSourceEvent.h"
#include "domain/core/angle/AngleSourcePacket.h"
#include "domain/core/calibration/strategy/CalibrationStrategyBeginContext.h"
#include "domain/core/calibration/strategy/CalibrationStrategyFeedContext.h"
#include "domain/core/drivers/motor/MotorDriverEvent.h"
#include "domain/core/pressure/PressureSourceEvent.h"
#include "domain/ports/angle/IAngleSource.h"
#include "domain/ports/calibration/recording/ICalibrationRecorder.h"
#include "domain/ports/calibration/strategy/ICalibrationStrategy.h"
#include "domain/ports/drivers/motor/IMotorDriver.h"
#include "domain/ports/pressure/IPressureSource.h"

using namespace application;
using namespace application::orchestrators;
using namespace domain::common;

namespace {
constexpr auto kMotorWatchdogTimeout = std::chrono::milliseconds(200);
constexpr auto kHomeTimeout = std::chrono::seconds(30);
constexpr auto kPressureWaitTimeout = std::chrono::seconds(120);
constexpr auto kPostZeroDelay = std::chrono::milliseconds(1500);
}

CalibrationOrchestrator::CalibrationOrchestrator(CalibrationOrchestratorPorts ports)
    : logger_(ports.logger)
    , ports_(std::move(ports))
    , inp_{}
    , safety_monitor_(logger_)
{
}

CalibrationOrchestrator::~CalibrationOrchestrator()
{
    stopInternal(ShutdownMode::Regular);
}

bool CalibrationOrchestrator::start(CalibrationOrchestratorInput input)
{
    logger_.info("CalibrationOrchestrator start requested");

    CalibrationOrchestratorState expected = CalibrationOrchestratorState::Stopped;
    if (!state_.compare_exchange_strong(
            expected,
            CalibrationOrchestratorState::Starting,
            std::memory_order_acq_rel))
    {
        return false;
    }

    inp_ = std::move(input);
    opened_angle_sources_.clear();

    try
    {
        // ---------- Motor ----------
        if (ports_.motor_driver.state() == MotorDriverState::Uninitialized)
        {
            if (!ports_.motor_driver.initialize())
                throw std::runtime_error(logger_.lastError());
        }
        else if (ports_.motor_driver.state() == MotorDriverState::Running)
        {
            ports_.motor_driver.stop();
        }

        // ---------- Cameras ----------
        const auto opened = ports_.source_manager.opened();
        if (opened.empty())
            throw std::runtime_error("No opened angle sources");

        for (const auto& id : opened)
        {
            auto src = ports_.source_storage.at(id);
            if (!src)
                throw std::runtime_error(
                    fmt::format("Opened angle source is missing in storage: {}", id.value));

            src->angle_source.start();
            opened_angle_sources_.insert(id);
        }

        // ---------- Observers ----------
        attachObservers();

        // ---------- Motor watchdog ----------
        ports_.motor_driver.watchdog().start(kMotorWatchdogTimeout);

        // ---------- Safety monitor ----------
        safety_monitor_.start(std::vector(opened_angle_sources_.begin(), opened_angle_sources_.end()));
        safety_monitor_.setMotorDirection(MotorDirection::Forward);
        if (!inp_.gauge.points.value.empty())
        {
            const auto max_pressure_it = std::max_element(
                inp_.gauge.points.value.begin(),
                inp_.gauge.points.value.end());

            const double half_scale = static_cast<double>(*max_pressure_it) * 0.5;
            safety_monitor_.setAngleTimeoutLowPressureThreshold(Pressure(half_scale, inp_.pressure_unit));
        }

        // ---------- Pressure ----------
        if (!ports_.pressure_source.isRunning())
        {
            if (!ports_.pressure_source.start())
                throw std::runtime_error(logger_.lastError());
        }

        // ---------- Strategy ----------
        CalibrationStrategyBeginContext ctx;
        ctx.pressure_unit    = inp_.pressure_unit;
        ctx.calibration_mode = inp_.calibration_mode;
        ctx.pressure_points  = PressurePoints::from(inp_.gauge.points.value, inp_.pressure_unit);
        ctx.slowdown_at_checkpoints = inp_.slowdown_at_checkpoints;
        ctx.play_valve = inp_.play_valve;

        const auto verdict = ports_.strategy.begin(ctx);
        const auto exec = applyVerdict(verdict);

        if (exec.fault)
            throw std::runtime_error(*exec.fault);

        if (exec.complete)
            throw std::runtime_error(
                "Calibration strategy protocol error: begin() returned Complete");

        // НАЧАТЬ ЗАПИСЬ
        {
            CalibrationLayout calibration_layout;

            calibration_layout.sources = std::vector(opened_angle_sources_.begin(), opened_angle_sources_.end());
            calibration_layout.directions.push_back(MotorDirection::Forward);

            calibration_layout.directions.push_back(MotorDirection::Backward);

            const auto& gauge_points = inp_.gauge.points.value;
            if (gauge_points.empty()) {
                throw std::runtime_error("Gauge has no pressure points");
            }

            if (inp_.calibration_mode == CalibrationMode::OnlyLast && gauge_points.size() >= 2) {
                calibration_layout.points.push_back(PointId(0, gauge_points.front()));
                calibration_layout.points.push_back(PointId(1, gauge_points.back()));
            } else {
                int i = 0;
                for (const auto& pp : gauge_points) {
                    calibration_layout.points.push_back(PointId(i, pp));
                    i++;
                }
            }

            CalibrationRecordingContext recording_context {
                calibration_layout,
                inp_.gauge,
                inp_.calibration_mode,
            };

            ports_.recorder.startRecording(recording_context);
        }

        state_.store(
            CalibrationOrchestratorState::Started,
            std::memory_order_release);

        ports_.session_clock.start();

        notifyObservers(
            CalibrationOrchestratorEvent(
                CalibrationOrchestratorEvent::Started{}));

        return true;
    }
    catch (const std::exception& e)
    {
        logger_.error("Calibration start failed: {}", e.what());

        teardown();

        state_.store(
            CalibrationOrchestratorState::Stopped,
            std::memory_order_release);

        CalibrationOrchestratorEvent::Failed ev;
        ev.error = e.what();
        notifyObservers(CalibrationOrchestratorEvent(ev));

        return false;
    }

    logger_.info("CalibrationOrchestrator starting");
}

void CalibrationOrchestrator::stop()
{
    stopInternal(ShutdownMode::UserStop);
}

void CalibrationOrchestrator::emergencyStop()
{
    stopInternal(ShutdownMode::EmergencyStop);
}

void CalibrationOrchestrator::stopInternal(ShutdownMode mode)
{
    auto current = state_.load(std::memory_order_acquire);

    if (current == CalibrationOrchestratorState::Stopped ||
        current == CalibrationOrchestratorState::Stopping)
    {
        return;
    }

    state_.store(CalibrationOrchestratorState::Stopping, std::memory_order_acq_rel);

    teardown(mode);

    state_.store(
        CalibrationOrchestratorState::Stopped,
        std::memory_order_release);

    notifyObservers(
        CalibrationOrchestratorEvent(
            CalibrationOrchestratorEvent::Stopped{}));
}

bool CalibrationOrchestrator::isRunning() const
{
    return state_.load(std::memory_order_acquire)
        == CalibrationOrchestratorState::Started;
}

void CalibrationOrchestrator::addObserver(ports::CalibrationOrchestratorObserver& observer)
{
    observers_.add(observer);
}

void CalibrationOrchestrator::removeObserver(ports::CalibrationOrchestratorObserver& observer)
{
    observers_.remove(observer);
}

void CalibrationOrchestrator::onPressureSourceEvent(const PressureSourceEvent& ev)
{
    std::string error_to_report;

    if (state_.load(std::memory_order_acquire) != CalibrationOrchestratorState::Started)
        return;

    std::visit(
        [&error_to_report](const auto& e)
        {
            using T = std::decay_t<decltype(e)>;

            if constexpr (std::is_same_v<T, PressureSourceEvent::Closed>)
            {
                error_to_report = "Pressure source closed";
            }
            else if constexpr (std::is_same_v<T, PressureSourceEvent::Failed>)
            {
                error_to_report = "Pressure source failed";
            }
        },
        ev.data);

    if (!error_to_report.empty())
        stopWithError(error_to_report);
}

void CalibrationOrchestrator::onPressurePacket(const PressurePacket& p)
{
    {
        std::lock_guard lock(pressure_mutex_);
        latest_pressure_pa_ = p.pressure.pa();
    }
    pressure_cv_.notify_all();

    if (state_.load(std::memory_order_acquire) != CalibrationOrchestratorState::Started)
        return;

    if (const auto incident = safety_monitor_.onPressurePacket(p))
    {
        stopWithError(incident->message);
        return;
    }

    CalibrationStrategyFeedContext ctx;
    ctx.timestamp = p.timestamp.asSeconds();
    ctx.pressure  = p.pressure.to(inp_.pressure_unit);
    ctx.limits_state = ports_.motor_driver.limits();

    ports_.motor_driver.watchdog().feed();
    const auto verdict = ports_.strategy.feed(ctx);
    const auto exec = applyVerdict(verdict);

    if (!exec.complete &&
        !exec.fault &&
        state_.load(std::memory_order_acquire) == CalibrationOrchestratorState::Started)
    {
        PressureSample sample;
        sample.time = p.timestamp.asSeconds();
        sample.pressure = p.pressure.to(inp_.pressure_unit);
        ports_.recorder.record(sample);
    }

    if (exec.complete)
    {
        logger_.info("Calibration strategy finished successfully.");
        stopInternal(ShutdownMode::StrategySuccess);
    }
    else if (exec.fault)
    {
        logger_.error("Calibration strategy failed: {}", *exec.fault);
        stopWithError(*exec.fault);
    }
}

void CalibrationOrchestrator::onAnglePacket(const AngleSourcePacket& p)
{
    if (state_.load(std::memory_order_acquire) != CalibrationOrchestratorState::Started)
        return;

    if (const auto incident = safety_monitor_.onAnglePacket(p))
    {
        stopWithError(incident->message);
        return;
    }

    AngleSample sample;
    sample.id = p.source_id;
    sample.time = p.timestamp.asSeconds();
    sample.angle = p.angle.to(inp_.angle_unit);

    ports_.recorder.record(sample);
}

void CalibrationOrchestrator::onAngleSourceEvent(const AngleSourceEvent& ev)
{
    std::string error_to_report;

    if (state_.load(std::memory_order_acquire) != CalibrationOrchestratorState::Started)
        return;

    std::visit(
        [&error_to_report](const auto& e)
        {
            using T = std::decay_t<decltype(e)>;

            if constexpr (std::is_same_v<T, AngleSourceEvent::Stopped>)
            {
                error_to_report = "Angle source stopped";
            }
            else if constexpr (std::is_same_v<T, AngleSourceEvent::Failed>)
            {
                error_to_report = e.error.error;
            }
        },
        ev.data);

    if (!error_to_report.empty())
        stopWithError(error_to_report);
}

void CalibrationOrchestrator::onMotorEvent(const MotorDriverEvent& ev)
{
    bool should_stop_successfully = false;
    std::string error_to_report;

    if (state_.load(std::memory_order_acquire) != CalibrationOrchestratorState::Started)
        return;

    std::visit(
        [&error_to_report, &should_stop_successfully](const auto& e)
        {
            using T = std::decay_t<decltype(e)>;

            if constexpr (std::is_same_v<T, MotorDriverEvent::Fault>)
            {
                error_to_report = e.error.message;
            }
            else if constexpr (std::is_same_v<T, MotorDriverEvent::StoppedAtHome>)
            {
                should_stop_successfully = true;
            }
        },
        ev.data);

    if (!error_to_report.empty())
        stopWithError(error_to_report);
    else if (should_stop_successfully)
    {
        logger_.info("Motor reported HOME limit reached, finishing calibration.");
        stop();
    }
}

void CalibrationOrchestrator::attachObservers()
{
    ports_.motor_driver.addObserver(*this);
    ports_.pressure_source.addObserver(*this);
    ports_.pressure_source.addSink(*this);

    for (const auto& id : opened_angle_sources_)
    {
        auto it = ports_.source_storage.at(id);
        if (!it)
        {
            throw std::runtime_error(
                fmt::format("Angle source disappeared during attach: {}", id.value));
        }

        it->angle_source.addObserver(*this);
        it->angle_source.addSink(*this);
    }
}

void CalibrationOrchestrator::detachObservers()
{
    ports_.motor_driver.removeObserver(*this);
    ports_.pressure_source.removeObserver(*this);
    ports_.pressure_source.removeSink(*this);

    for (const auto& id : opened_angle_sources_)
    {
        auto it = ports_.source_storage.at(id);
        if (it)
        {
            it->angle_source.removeObserver(*this);
            it->angle_source.removeSink(*this);
        }
    }
}

void CalibrationOrchestrator::notifyObservers(const CalibrationOrchestratorEvent& ev)
{
    observers_.notify(
        [&ev](ports::CalibrationOrchestratorObserver& o)
        {
            o.onCalibrationOrchestratorEvent(ev);
        });
}

void CalibrationOrchestrator::teardown(ShutdownMode mode)
{
    safety_monitor_.stop();
    ports_.motor_driver.watchdog().stop();

    // Сначала даём стратегии корректно завершиться и вернуть shutdown-команды.
    if (mode != ShutdownMode::EmergencyStop && ports_.strategy.isRunning())
    {
        const auto end_verdict = ports_.strategy.end();
        const auto exec = applyVerdict(end_verdict);

        if (exec.fault)
            logger_.warn("Strategy end() reported fault: {}", *exec.fault);
    }

    if (mode == ShutdownMode::UserStop)
    {
        performUserStopSequence();
    }
    else if (mode == ShutdownMode::StrategySuccess)
    {
        performSuccessSequence();
    }
    else if (mode == ShutdownMode::EmergencyStop)
    {
        ports_.motor_driver.emergencyStop();
    }
    else
    {
        ports_.motor_driver.stop();
    }

    detachObservers();
    ports_.pressure_source.stop();

    for (const auto& id : opened_angle_sources_)
    {
        auto src = ports_.source_storage.at(id);
        if (src) src->angle_source.stop();
    }

    opened_angle_sources_.clear();
    ports_.session_clock.stop();

    // ОСТАНОВИТЬ ЗАПИСЬ
    ports_.recorder.stopRecording();
}

void CalibrationOrchestrator::stopWithError(const std::string& error)
{
    auto current = state_.load(std::memory_order_acquire);
    if (current != CalibrationOrchestratorState::Starting &&
        current != CalibrationOrchestratorState::Started)
    {
        return;
    }

    CalibrationOrchestratorState expected = current;
    if (!state_.compare_exchange_strong(
            expected,
            CalibrationOrchestratorState::Stopping,
            std::memory_order_acq_rel))
    {
        return;
    }

    teardown(ShutdownMode::Error);

    state_.store(
        CalibrationOrchestratorState::Stopped,
        std::memory_order_release);

    CalibrationOrchestratorEvent::Failed ev;
    ev.error = error;
    notifyObservers(CalibrationOrchestratorEvent(ev));
}

CalibrationOrchestrator::StrategyExecutionResult
CalibrationOrchestrator::applyVerdict(const StrategyVerdict& verdict)
{
    StrategyExecutionResult result;

    for (const auto& command : verdict.commands)
    {
        std::visit(
            [this, &result](const auto& c)
            {
                using T = std::decay_t<decltype(c)>;

                if constexpr (std::is_same_v<T, StrategyVerdict::Complete>)
                {
                    result.complete = true;
                }
                else if constexpr (std::is_same_v<T, StrategyVerdict::Fault>)
                {
                    result.fault = c.error;
                }
                else
                {
                    applyCommand(c);
                }
            },
            command);

        if (result.complete || result.fault)
            break;
    }

    return result;
}

void CalibrationOrchestrator::applyCommand(const StrategyVerdict::BeginSession& cmd)
{
    ports_.recorder.beginSession(cmd.id);
}

void CalibrationOrchestrator::applyCommand(const StrategyVerdict::EndSession&)
{
    ports_.recorder.endSession();
}

void CalibrationOrchestrator::applyCommand(const StrategyVerdict::MotorSetFrequency& cmd)
{
    ports_.motor_driver.setFrequency(MotorFrequency(cmd.freq));
}

void CalibrationOrchestrator::applyCommand(const StrategyVerdict::MotorSetDirection& cmd)
{
    ports_.motor_driver.setDirection(cmd.direction);
    safety_monitor_.setMotorDirection(cmd.direction);
}

void CalibrationOrchestrator::applyCommand(const StrategyVerdict::MotorSetFlaps& cmd)
{
    ports_.motor_driver.setFlapsState(cmd.state);
}

void CalibrationOrchestrator::applyCommand(const StrategyVerdict::MotorStart&)
{
    ports_.motor_driver.start();
}

void CalibrationOrchestrator::applyCommand(const StrategyVerdict::MotorStop&)
{
    ports_.motor_driver.stop();
}

void CalibrationOrchestrator::applyCommand(const StrategyVerdict::StatusText& cmd)
{
    notifyObservers(CalibrationOrchestratorEvent(CalibrationOrchestratorEvent::StatusText{cmd.text}));
}

void CalibrationOrchestrator::applyCommand(const StrategyVerdict::Complete&)
{
    stopInternal(ShutdownMode::StrategySuccess);
}

void CalibrationOrchestrator::performUserStopSequence()
{
    moveMotorToHome();
    depressurizeAndCloseFlaps();
}

void CalibrationOrchestrator::performSuccessSequence()
{
    depressurizeAndCloseFlaps();
}

void CalibrationOrchestrator::moveMotorToHome()
{
    logger_.info("Manual stop: moving motor to HOME position.");

    ports_.motor_driver.setDirection(MotorDirection::Backward);
    ports_.motor_driver.setFrequency(MotorFrequency(2000));
    ports_.motor_driver.start();

    const auto started = std::chrono::steady_clock::now();
    while (!ports_.motor_driver.limits().home)
    {
        if (std::chrono::steady_clock::now() - started >= kHomeTimeout)
        {
            logger_.warn("Manual stop: HOME wasn't reached within timeout.");
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    ports_.motor_driver.stop();
}

void CalibrationOrchestrator::depressurizeAndCloseFlaps()
{
    logger_.info("Opening exhaust flap and waiting pressure <= 0.");
    ports_.motor_driver.setFlapsState(MotorFlapsState::ExhaustOpened);

    if (!waitForPressureAtOrBelowZero(kPressureWaitTimeout))
    {
        logger_.warn("Timeout while waiting pressure <= 0. Closing flaps anyway.");
    }
    else
    {
        std::this_thread::sleep_for(kPostZeroDelay);
    }

    ports_.motor_driver.setFlapsState(MotorFlapsState::FlapsClosed);
}

bool CalibrationOrchestrator::waitForPressureAtOrBelowZero(std::chrono::seconds timeout)
{
    std::unique_lock lock(pressure_mutex_);
    return pressure_cv_.wait_for(
        lock,
        timeout,
        [this]
        {
            return latest_pressure_pa_.has_value() && *latest_pressure_pa_ <= 0.0;
        });
}
