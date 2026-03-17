#include "InMemoryCalibrationRecorder.h"

#include <algorithm>

#include "domain/ports/calibration/recording/ICalibrationRecorderObserver.h"

using namespace infra::calib;
using namespace domain::common;
using namespace domain::ports;

InMemoryCalibrationRecorder::InMemoryCalibrationRecorder(
    CalibrationRecorderPorts ports,
    InMemoryCalibrationRecorderConfig config
)
    : logger_(ports.logger)
    , config_(std::move(config))
{
}

void InMemoryCalibrationRecorder::startRecording(CalibrationLayout layout)
{
    if (recording_active_)
        return;

    recording_active_ = true;

    logger_.info("Calibration recording started.");

    CalibrationRecorderEvent::RecordingStarted e;
    e.layout = layout;
    e.gauge =
    notify(CalibrationRecorderEvent(e));
}

void InMemoryCalibrationRecorder::stopRecording()
{
    if (!recording_active_)
        return;

    if (active_session_)
        endSession();

    recording_active_ = false;

    logger_.info("Calibration recording stopped.");

    notify(CalibrationRecorderEvent(
        CalibrationRecorderEvent::RecordingStopped{}));
}


void InMemoryCalibrationRecorder::beginSession(CalibrationSessionId id)
{
    if (active_session_)
    {
        logger_.warn(
            "beginSession() called while another session is active. "
            "Previous session will be closed automatically.");
        endSession();
    }

    CalibrationSession new_session;
    new_session.id = id;

    active_session_ = std::move(new_session);

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
    if (active_session_)
    {
        active_session_->pressure_series.push(sample.time, sample.pressure);
    }

    CalibrationRecorderEvent::PressureSampleRecorded ev;
    ev.sample = sample;

    notify(CalibrationRecorderEvent(ev));
}

void InMemoryCalibrationRecorder::record(const AngleSample& sample)
{
    if (active_session_)
    {
        active_session_->angle_series[sample.id].push(sample.time, sample.angle);
    }

    CalibrationRecorderEvent::AngleSampleRecorded ev;
    ev.sample = sample;

    notify(CalibrationRecorderEvent(ev));
}

void InMemoryCalibrationRecorder::endSession()
{
    if (!active_session_)
    {
        logger_.warn(
            "endSession() called, but there is no active calibration session.");
        return;
    }

    const auto& session = *active_session_;

    const std::size_t pressure_count = session.pressure_series.size();

    std::size_t angle_count = 0;
    for (const auto& [id, series] : session.angle_series)
    {
        (void)id;
        angle_count += series.size();
    }

    logger_.info(
        "Calibration session finished, captured {} pressures and {} angles.",
        pressure_count,
        angle_count
    );

    sessions_[session.id] = session;

    CalibrationRecorderEvent::SessionEnded ev;
    ev.id = session.id;
    ev.result = session;

    notify(CalibrationRecorderEvent(ev));

    active_session_.reset();
}

std::vector<CalibrationSessionId>
InMemoryCalibrationRecorder::sessions() const
{
    std::vector<CalibrationSessionId> result;

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

    return result;
}

std::optional<CalibrationSession>
InMemoryCalibrationRecorder::session(CalibrationSessionId id) const
{
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