#include "InMemoryCalibrationRecorder.h"

using namespace infra::calib;
using namespace domain::common;

InMemoryCalibrationRecorder::InMemoryCalibrationRecorder(
    CalibrationRecorderPorts ports,
    InMemoryCalibrationRecorderConfig config
)
    : logger_(ports.logger)
    , config_(std::move(config)) {
}

void InMemoryCalibrationRecorder::beginSession(CalibrationSessionId id) {
    if (active_session_.has_value()) {
        logger_.warn("beginSession() called while another session is active. Previous session will be closed automatically.");
        endSession();
    }

    CalibrationSession new_session;
    new_session.id = id;

    active_session_ = std::move(new_session);

    logger_.info("Calibration {}/{} session started.", id.point.id, id.direction);
}

void InMemoryCalibrationRecorder::record(const PressureSample& sample) {
    if (!active_session_.has_value()) {
        return;
    }

    active_session_->pressure_series.push(sample.time, sample.pressure);
}

void InMemoryCalibrationRecorder::record(const AngleSample& sample) {
    if (!active_session_.has_value()) {
        return;
    }

    active_session_->angle_series[sample.id].push(sample.time, sample.angle);
}

void InMemoryCalibrationRecorder::endSession() {
    if (!active_session_.has_value()) {
        logger_.warn("endSession() called, but there is no active calibration session.");
        return;
    }

    const auto& session = *active_session_;

    const std::size_t pressure_count = session.pressure_series.size();

    std::size_t angle_count = 0;
    for (const auto& [id, series] : session.angle_series) {
        (void)id;
        angle_count += series.size();
    }

    logger_.info(
        "Calibration session finished, captured {} pressures and {} angles.",
        pressure_count,
        angle_count
    );

    sessions_[session.id] = session;
    active_session_.reset();
}

std::vector<CalibrationSessionId> InMemoryCalibrationRecorder::sessions() const {
    std::vector<CalibrationSessionId> result;
    result.reserve(sessions_.size() + (active_session_.has_value() ? 1U : 0U));

    for (const auto& [id, session] : sessions_) {
        (void)session;
        result.push_back(id);
    }

    if (active_session_.has_value()) {
        const auto already_saved = sessions_.find(active_session_->id) != sessions_.end();
        if (!already_saved) {
            result.push_back(active_session_->id);
        }
    }

    return result;
}

std::optional<CalibrationSession> InMemoryCalibrationRecorder::session(CalibrationSessionId id) const {
    if (active_session_.has_value() && active_session_->id == id) {
        return active_session_;
    }

    const auto it = sessions_.find(id);
    if (it != sessions_.end()) {
        return it->second;
    }

    return std::nullopt;
}