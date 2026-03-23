#include "CalibrationOrchestrator.h"

#include <chrono>
#include <stdexcept>
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
}

CalibrationOrchestrator::CalibrationOrchestrator(CalibrationOrchestratorPorts ports)
    : logger_(ports.logger)
    , ports_(std::move(ports))
    , inp_{}
{
}

CalibrationOrchestrator::~CalibrationOrchestrator()
{
    stop();
}

bool CalibrationOrchestrator::start(CalibrationOrchestratorInput input)
{
    std::lock_guard<std::mutex> lock(lifecycle_mutex_);

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

    CompensationStack rollback;

    try
    {
        if (auto r = prepareMotor(rollback); !r)
            throw std::runtime_error(r.error().message);

        if (auto r = prepareSources(rollback); !r)
            throw std::runtime_error(r.error().message);

        if (auto r = attachObserversPhase(rollback); !r)
            throw std::runtime_error(r.error().message);

        if (auto r = startPressureSource(rollback); !r)
            throw std::runtime_error(r.error().message);

        if (auto r = beginStrategy(rollback); !r)
            throw std::runtime_error(r.error().message);

        if (auto r = startRecording(rollback); !r)
            throw std::runtime_error(r.error().message);

        if (auto r = publishStarted(); !r)
            throw std::runtime_error(r.error().message);

        return true;
    }
    catch (const std::exception& e)
    {
        logger_.error("Calibration start failed: {}", e.what());

        rollbackStart(rollback);

        state_.store(
            CalibrationOrchestratorState::Stopped,
            std::memory_order_release);

        CalibrationOrchestratorEvent::Failed ev;
        ev.error = e.what();
        notifyObservers(CalibrationOrchestratorEvent(ev));

        return false;
    }
}

void CalibrationOrchestrator::stop()
{
    std::lock_guard<std::mutex> lock(lifecycle_mutex_);

    auto current = state_.load(std::memory_order_acquire);

    if (current == CalibrationOrchestratorState::Stopped ||
        current == CalibrationOrchestratorState::Stopping)
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

    teardown();

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
    if (state_.load(std::memory_order_acquire) != CalibrationOrchestratorState::Started)
        return;

    CalibrationStrategyFeedContext ctx;
    ctx.timestamp = p.timestamp.asSeconds();
    ctx.pressure = p.pressure.to(inp_.pressure_unit);
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
        stop();
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
    std::string error_to_report;

    if (state_.load(std::memory_order_acquire) != CalibrationOrchestratorState::Started)
        return;

    std::visit(
        [&error_to_report](const auto& e)
        {
            using T = std::decay_t<decltype(e)>;

            if constexpr (std::is_same_v<T, MotorDriverEvent::Fault>)
            {
                error_to_report = e.error.message;
            }
        },
        ev.data);

    if (!error_to_report.empty())
        stopWithError(error_to_report);
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

void CalibrationOrchestrator::teardown()
{
    ports_.motor_driver.stop();
    detachObservers();

    if (ports_.strategy.isRunning())
    {
        const auto end_verdict = ports_.strategy.end();
        const auto exec = applyVerdict(end_verdict);

        if (exec.fault)
            logger_.warn("Strategy end() reported fault: {}", *exec.fault);
    }
    else
    {
        ports_.motor_driver.stop();
    }

    ports_.pressure_source.stop();

    for (const auto& id : opened_angle_sources_)
    {
        auto src = ports_.source_storage.at(id);
        if (src)
            src->angle_source.stop();
    }

    opened_angle_sources_.clear();
    ports_.session_clock.stop();
    ports_.recorder.stopRecording();
}

void CalibrationOrchestrator::stopWithError(const std::string& error)
{
    std::lock_guard<std::mutex> lock(lifecycle_mutex_);

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

    teardown();

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

CalibrationOrchestrator::StartResult
CalibrationOrchestrator::prepareMotor(CompensationStack& rollback)
{
    if (ports_.motor_driver.state() == MotorDriverState::Uninitialized)
    {
        if (!ports_.motor_driver.initialize())
        {
            return makeStartFailure("Motor driver initialization failed");
        }
    }
    else if (ports_.motor_driver.state() == MotorDriverState::Running)
    {
        ports_.motor_driver.stop();
    }

    pushRollback(
        rollback,
        [this]()
        {
            ports_.motor_driver.stop();
        });

    return StartResult::success();
}

CalibrationOrchestrator::StartResult
CalibrationOrchestrator::prepareSources(CompensationStack& rollback)
{
    const auto opened = ports_.source_manager.opened();
    if (opened.empty())
    {
        return makeStartFailure("No opened angle sources");
    }

    for (const auto& id : opened)
    {
        auto src = ports_.source_storage.at(id);
        if (!src)
        {
            return makeStartFailure(
                fmt::format("Opened angle source is missing in storage: {}", id.value));
        }

        src->angle_source.start();
        opened_angle_sources_.insert(id);

        pushRollback(
            rollback,
            [this, id]()
            {
                auto src = ports_.source_storage.at(id);
                if (src)
                    src->angle_source.stop();
            });
    }

    pushRollback(
        rollback,
        [this]()
        {
            opened_angle_sources_.clear();
        });

    return StartResult::success();
}

CalibrationOrchestrator::StartResult
CalibrationOrchestrator::attachObserversPhase(CompensationStack& rollback)
{
    attachObservers();

    pushRollback(
        rollback,
        [this]()
        {
            detachObservers();
        });

    return StartResult::success();
}

CalibrationOrchestrator::StartResult
CalibrationOrchestrator::startPressureSource(CompensationStack& rollback)
{
    if (!ports_.pressure_source.isRunning())
    {
        if (!ports_.pressure_source.start())
        {
            return makeStartFailure("Pressure source start failed");
        }

        pushRollback(
            rollback,
            [this]()
            {
                ports_.pressure_source.stop();
            });
    }

    return StartResult::success();
}

CalibrationOrchestrator::StartResult
CalibrationOrchestrator::beginStrategy(CompensationStack& rollback)
{
    CalibrationStrategyBeginContext ctx;
    ctx.pressure_unit = inp_.pressure_unit;
    ctx.calibration_mode = inp_.calibration_mode;
    ctx.pressure_points = PressurePoints::from(inp_.gauge.points.value, inp_.pressure_unit);

    const auto verdict = ports_.strategy.begin(ctx);
    const auto exec = applyVerdict(verdict);

    if (exec.fault)
    {
        return makeStartFailure(*exec.fault);
    }

    if (exec.complete)
    {
        return makeStartFailure(
            "Calibration strategy protocol error: begin() returned Complete");
    }

    pushRollback(
        rollback,
        [this]()
        {
            if (!ports_.strategy.isRunning())
                return;

            const auto end_verdict = ports_.strategy.end();
            const auto exec = applyVerdict(end_verdict);

            if (exec.fault)
                logger_.warn("Strategy end() reported fault during rollback: {}", *exec.fault);
        });

    return StartResult::success();
}

CalibrationOrchestrator::StartResult
CalibrationOrchestrator::startRecording(CompensationStack& rollback)
{
    const auto calibration_layout = layout_builder_.build(inp_, opened_angle_sources_);

    CalibrationRecordingContext recording_context {
        calibration_layout,
        inp_.gauge,
    };

    ports_.recorder.startRecording(recording_context);

    pushRollback(
        rollback,
        [this]()
        {
            ports_.recorder.stopRecording();
        });

    return StartResult::success();
}

CalibrationOrchestrator::StartResult
CalibrationOrchestrator::publishStarted()
{
    ports_.motor_driver.watchdog().start(kMotorWatchdogTimeout);
    ports_.session_clock.start();

    state_.store(
        CalibrationOrchestratorState::Started,
        std::memory_order_release);

    try
    {
        notifyObservers(
            CalibrationOrchestratorEvent(
                CalibrationOrchestratorEvent::Started{}));
    }
    catch (const std::exception& e)
    {
        logger_.error("Failed to notify observers about start: {}", e.what());
    }
    catch (...)
    {
        logger_.error("Failed to notify observers about start: unknown error");
    }

    logger_.info("CalibrationOrchestrator started");

    return StartResult::success();
}

void CalibrationOrchestrator::rollbackStart(CompensationStack& rollback) noexcept
{
    for (auto it = rollback.rbegin(); it != rollback.rend(); ++it)
    {
        try
        {
            (*it)();
        }
        catch (const std::exception& e)
        {
            logger_.warn("Rollback action failed: {}", e.what());
        }
        catch (...)
        {
            logger_.warn("Rollback action failed: unknown error");
        }
    }

    rollback.clear();
    opened_angle_sources_.clear();
    ports_.session_clock.stop();
    ports_.recorder.stopRecording();
}

void CalibrationOrchestrator::pushRollback(
    CompensationStack& rollback,
    CompensationAction action)
{
    rollback.push_back(std::move(action));
}

CalibrationOrchestrator::StartResult
CalibrationOrchestrator::makeStartFailure(std::string message)
{
    return StartResult::failure(CalibrationStartError{std::move(message)});
}
