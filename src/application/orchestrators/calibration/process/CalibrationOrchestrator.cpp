#include "CalibrationOrchestrator.h"

#include <algorithm>
#include <stdexcept>
#include <variant>
#include <vector>

#include "application/orchestrators/calibration/process/CalibrationOrchestratorEvent.h"
#include "application/orchestrators/video/VideoSourceManager.h"
#include "application/ports/calibration/orchestration/CalibrationOrchestratorObserver.h"
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

using namespace application::orchestrators;
using namespace domain::common;

CalibrationOrchestrator::CalibrationOrchestrator(CalibrationOrchestratorPorts ports)
    : logger_(ports.logger)
    , ports_(ports)
    , inp_{}
{
}

CalibrationOrchestrator::~CalibrationOrchestrator()
{
    stop();
}

bool CalibrationOrchestrator::start(CalibrationOrchestratorInput input)
{
    CalibrationOrchestratorState expected = CalibrationOrchestratorState::Stopped;
    if (!state_.compare_exchange_strong(expected,
                                        CalibrationOrchestratorState::Starting,
                                        std::memory_order_acq_rel))
    {
        return false;
    }

    inp_ = input;
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
                throw std::runtime_error(fmt::format("Opened angle source is missing in storage: {}", id.value));
            src->angle_source.start();

            opened_angle_sources_.insert(id);
        }

        // ---------- Observers ----------
        attachObservers();

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
        ctx.pressure_points  = inp_.pressure_points;

        ports_.strategy.bind(ports_.motor_driver, ports_.recorder);
        const auto verdict = ports_.strategy.begin(ctx);

        std::visit(
            [this](const auto& e)
            {
                using T = std::decay_t<decltype(e)>;

                if constexpr (std::is_same_v<T, CalibrationStrategyVerdict::Fault>)
                {
                    logger_.error("Calibration strategy begin failed: {}", e.error);
                    throw std::runtime_error(e.error);
                }
            },
            verdict.data);

        state_.store(CalibrationOrchestratorState::Started,
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

        // rollback из Starting / partially-started
        teardown();

        state_.store(CalibrationOrchestratorState::Stopped,
                     std::memory_order_release);

        CalibrationOrchestratorEvent::Failed ev;
        ev.error = e.what();
        notifyObservers(CalibrationOrchestratorEvent(ev));

        return false;
    }
}

void CalibrationOrchestrator::stop()
{
    CalibrationOrchestratorState current = state_.load(std::memory_order_acquire);

    if (current == CalibrationOrchestratorState::Stopped ||
        current == CalibrationOrchestratorState::Stopping)
    {
        return;
    }

    CalibrationOrchestratorState expected = current;
    if (!state_.compare_exchange_strong(expected,
                                        CalibrationOrchestratorState::Stopping,
                                        std::memory_order_acq_rel))
    {
        return;
    }

    teardown();

    state_.store(CalibrationOrchestratorState::Stopped,
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
    enum class Action
    {
        None,
        Complete,
        Fail
    };

    Action action = Action::None;
    std::string error;

    if (state_.load(std::memory_order_acquire) != CalibrationOrchestratorState::Started)
        return;

    CalibrationStrategyFeedContext ctx;
    ctx.timestamp = p.timestamp.asSeconds();
    ctx.pressure  = p.pressure.to(inp_.pressure_unit);

    const auto verdict = ports_.strategy.feed(ctx);

    std::visit(
        [this, &action, &error](const auto& v)
        {
            using T = std::decay_t<decltype(v)>;

            if constexpr (std::is_same_v<T, CalibrationStrategyVerdict::Complete>)
            {
                logger_.info("Calibration strategy finished successfully.");
                action = Action::Complete;
            }
            else if constexpr (std::is_same_v<T, CalibrationStrategyVerdict::Fault>)
            {
                logger_.error("Calibration strategy failed: {}", v.error);
                action = Action::Fail;
                error = v.error;
            }
        },
        verdict.data);

    if (action == Action::None &&
        state_.load(std::memory_order_acquire) == CalibrationOrchestratorState::Started)
    {
        PressureSample sample;
        sample.time = p.timestamp.asSeconds();
        sample.pressure = p.pressure.to(inp_.pressure_unit);
        ports_.recorder.record(sample);
    }

    if (action == Action::Complete)
    {
        stop();
    }
    else if (action == Action::Fail)
    {
        stopWithError(error);
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
            throw std::runtime_error(fmt::format("Angle source disappeared during attach: {}", id.value));

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
            it->angle_source.removeObserver(*this);
            it->angle_source.removeSink(*this);
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
    detachObservers();

    ports_.motor_driver.stop();
    ports_.pressure_source.stop();

    if (ports_.strategy.isRunning())
        ports_.strategy.end();
    ports_.session_clock.stop();
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
    if (!state_.compare_exchange_strong(expected,
                                        CalibrationOrchestratorState::Stopping,
                                        std::memory_order_acq_rel))
    {
        return;
    }

    teardown();

    state_.store(CalibrationOrchestratorState::Stopped,
                 std::memory_order_release);

    CalibrationOrchestratorEvent::Failed ev;
    ev.error = error;

    notifyObservers(CalibrationOrchestratorEvent(ev));
}