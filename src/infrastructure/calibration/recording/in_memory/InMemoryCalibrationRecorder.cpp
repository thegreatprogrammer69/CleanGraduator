#include "InMemoryCalibrationRecorder.h"

#include <algorithm>

#include "domain/ports/calibration/recording/ICalibrationRecorderObserver.h"

using namespace infra::calib;
using namespace domain::common;
using namespace domain::ports;

InMemoryCalibrationRecorder::InMemoryCalibrationRecorder(CalibrationRecorderPorts ports)
    : logger_(ports.logger)
{
}

void InMemoryCalibrationRecorder::startRecording(CalibrationRecordingContext ctx)
{
    {
        std::lock_guard lock(mutex_);
        if (recording_active_)
            return;

        recording_active_ = true;

        angle_counts_.clear();
        last_direction_.reset();
    }

    logger_.info("Calibration recording started.");

    CalibrationRecorderEvent::RecordingStarted e;
    e.layout = ctx.layout;
    e.gauge = ctx.gauge;
    e.calibration_mode = ctx.calibration_mode;
    notify(CalibrationRecorderEvent(e));
}

void InMemoryCalibrationRecorder::stopRecording()
{
    bool should_end_session = false;
    {
        std::lock_guard lock(mutex_);
        if (!recording_active_)
            return;
        should_end_session = active_session_.has_value();
    }

    if (should_end_session)
        endSession();

    {
        std::lock_guard lock(mutex_);
        recording_active_ = false;
    }

    logger_.info("Calibration recording stopped.");

    notify(CalibrationRecorderEvent(
        CalibrationRecorderEvent::RecordingStopped{}));
}


void InMemoryCalibrationRecorder::beginSession(CalibrationSessionId id)
{
    bool has_active_session = false;
    {
        std::lock_guard lock(mutex_);
        has_active_session = active_session_.has_value();
    }

    if (has_active_session)
    {
        logger_.warn(
            "beginSession() called while another session is active. "
            "Previous session will be closed automatically.");
        endSession();
    }

    CalibrationSession new_session;
    new_session.id = id;

    {
        std::lock_guard lock(mutex_);
        active_session_ = std::move(new_session);
        last_direction_ = id.direction;
    }

    logger_.info(
        "Calibration {}/{} session started.",
        id.point.id,
        id.direction);

    CalibrationRecorderEvent::SessionStarted ev;
    ev.id = id;

    notify(CalibrationRecorderEvent(ev));
}

void InMemoryCalibrationRecorder::record(const PressureSample& sample)
{
    {
        std::lock_guard lock(mutex_);
        if (active_session_)
        {
            active_session_->pressure_series.push(sample.time, sample.pressure);
        }
    }

    CalibrationRecorderEvent::PressureSampleRecorded ev;
    ev.sample = sample;

    notify(CalibrationRecorderEvent(ev));
}

void InMemoryCalibrationRecorder::record(const AngleSample& sample)
{
    {
        std::lock_guard lock(mutex_);
        if (active_session_)
        {
            active_session_->angle_series[sample.id].push(sample.time, sample.angle);
        }

        if (last_direction_)
        {
            ++angle_counts_[sample.id][*last_direction_];
        }
    }

    CalibrationRecorderEvent::AngleSampleRecorded ev;
    ev.sample = sample;

    notify(CalibrationRecorderEvent(ev));
}

void InMemoryCalibrationRecorder::endSession()
{
    std::optional<CalibrationSession> session;
    {
        std::lock_guard lock(mutex_);
        if (!active_session_)
        {
            logger_.warn(
                "endSession() called, but there is no active calibration session.");
            return;
        }
        session = active_session_;
    }

    const std::size_t pressure_count = session->pressure_series.size();

    std::size_t angle_count = 0;
    for (const auto& [id, series] : session->angle_series)
    {
        (void)id;
        angle_count += series.size();
    }

    logger_.info(
        "Calibration session finished, captured {} pressures and {} angles.",
        pressure_count,
        angle_count
    );

    {
        std::lock_guard lock(mutex_);
        sessions_[session->id] = *session;
    }

    CalibrationRecorderEvent::SessionEnded ev;
    ev.id = session->id;
    ev.result = *session;

    notify(CalibrationRecorderEvent(ev));

    {
        std::lock_guard lock(mutex_);
        active_session_.reset();
    }
}

std::vector<CalibrationSessionId>
InMemoryCalibrationRecorder::sessions() const
{
    std::vector<CalibrationSessionId> result;
    {
        std::lock_guard lock(mutex_);
        result.reserve(
            sessions_.size() +
            (active_session_ ? 1U : 0U));

        for (const auto& [id, session] : sessions_)
        {
            (void)session;
            result.push_back(id);
        }

        if (active_session_)
        {
            const bool already_saved =
                sessions_.find(active_session_->id) != sessions_.end();

            if (!already_saved)
                result.push_back(active_session_->id);
        }
    }

    return result;
}

std::optional<CalibrationSession>
InMemoryCalibrationRecorder::session(CalibrationSessionId id) const
{
    std::lock_guard lock(mutex_);
    if (active_session_ && active_session_->id == id)
        return active_session_;

    const auto it = sessions_.find(id);

    if (it != sessions_.end())
        return it->second;

    return std::nullopt;
}

void InMemoryCalibrationRecorder::addObserver(
    ICalibrationRecorderObserver& observer)
{
    observers_.add(observer);
}

void InMemoryCalibrationRecorder::removeObserver(
    ICalibrationRecorderObserver& observer)
{
    observers_.remove(observer);
}

void InMemoryCalibrationRecorder::notify(
    const CalibrationRecorderEvent& ev)
{
    observers_.notify(
        [&ev](ICalibrationRecorderObserver& o)
        {
            o.onCalibrationRecorderEvent(ev);
        });
}
domain::ports::CalibrationAngleCounts InMemoryCalibrationRecorder::angleCounts() const
{
    std::lock_guard lock(mutex_);
    return angle_counts_;
}
