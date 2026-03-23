#include "CalibrationSafetyMonitor.h"

#include <cmath>
#include <sstream>

namespace application::orchestrators {
namespace {
int makeSourceKey(const domain::common::SourceId& id)
{
    return id.value;
}
} // namespace

CalibrationSafetyMonitor::CalibrationSafetyMonitor()
    : CalibrationSafetyMonitor(Config{})
{
}

CalibrationSafetyMonitor::CalibrationSafetyMonitor(Config config)
    : config_(config)
{
}

CalibrationSafetyMonitor::~CalibrationSafetyMonitor()
{
    stop();
}

void CalibrationSafetyMonitor::start(const std::set<domain::common::SourceId>& source_ids)
{
    std::lock_guard<std::mutex> lock(mutex_);

    source_states_.clear();
    for (const auto& source_id : source_ids)
    {
        auto& state = source_states_[makeSourceKey(source_id)];
        state.previous_sample.reset();
        state.last_sample.reset();
        state.watchdog.start(config_.angle_source_silence_timeout);
    }
}

void CalibrationSafetyMonitor::stop()
{
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto& [_, state] : source_states_)
    {
        state.watchdog.stop();
    }

    source_states_.clear();
}

void CalibrationSafetyMonitor::onAnglePacket(const domain::common::AngleSourcePacket& packet)
{
    std::lock_guard<std::mutex> lock(mutex_);

    const auto it = source_states_.find(makeSourceKey(packet.source_id));
    if (it == source_states_.end())
    {
        return;
    }

    auto& state = it->second;
    state.watchdog.feed();

    AngleSample sample;
    sample.timestamp = packet.timestamp;
    sample.angle_deg = packet.angle.degrees();

    state.previous_sample = state.last_sample;
    state.last_sample = sample;
}

std::optional<CalibrationSafetyMonitor::Incident> CalibrationSafetyMonitor::detectIncident() const
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (const auto silence = detectSilenceIncidentLocked())
    {
        return silence;
    }

    for (const auto& [source_key, state] : source_states_)
    {
        const domain::common::SourceId source_id(source_key);

        if (const auto growth = detectAngleGrowthIncidentLocked(source_id, state))
        {
            return growth;
        }
    }

    return std::nullopt;
}

std::optional<CalibrationSafetyMonitor::Incident>
CalibrationSafetyMonitor::detectSilenceIncidentLocked() const
{
    for (const auto& [source_key, state] : source_states_)
    {
        if (!state.watchdog.expired())
        {
            continue;
        }

        const domain::common::SourceId source_id(source_key);

        Incident incident;
        incident.code = IncidentCode::AngleSourceSilence;
        incident.source_id = source_id;
        std::ostringstream oss;
        oss << "Angle source '" << source_key
            << "' produced no angle packets for more than "
            << config_.angle_source_silence_timeout.count() << " ms";
        incident.message = oss.str();

        if (state.last_sample)
        {
            incident.last_angle_deg = state.last_sample->angle_deg;
        }

        return incident;
    }

    return std::nullopt;
}

std::optional<CalibrationSafetyMonitor::Incident>
CalibrationSafetyMonitor::detectAngleGrowthIncidentLocked(
    const domain::common::SourceId& source_id,
    const SourceState& state) const
{
    if (!state.previous_sample || !state.last_sample)
    {
        return std::nullopt;
    }

    const auto dt = state.last_sample->timestamp.toDuration() - state.previous_sample->timestamp.toDuration();
    const auto dt_seconds = std::chrono::duration<double>(dt).count();
    if (dt_seconds <= 0.0)
    {
        return std::nullopt;
    }

    const auto delta_angle_deg = state.last_sample->angle_deg - state.previous_sample->angle_deg;
    if (delta_angle_deg <= 0.0)
    {
        return std::nullopt;
    }

    const auto angle_growth_rate_deg_per_sec = delta_angle_deg / dt_seconds;
    if (angle_growth_rate_deg_per_sec < config_.max_safe_angle_growth_rate_deg_per_sec)
    {
        return std::nullopt;
    }

    const auto remaining_angle_deg = config_.max_safe_angle_deg - state.last_sample->angle_deg;
    if (remaining_angle_deg <= 0.0)
    {
        Incident incident;
        incident.code = IncidentCode::AngleGrowthTooFast;
        incident.source_id = source_id;
        incident.last_angle_deg = state.last_sample->angle_deg;
        incident.angle_growth_rate_deg_per_sec = angle_growth_rate_deg_per_sec;
        incident.projected_limit_crossing_time_ms = 0.0;
        std::ostringstream oss;
        oss.setf(std::ios::fixed);
        oss.precision(2);
        oss << "Angle source '" << source_id.value
            << "' already reached unsafe angle " << state.last_sample->angle_deg
            << "° with growth rate " << angle_growth_rate_deg_per_sec << "°/s";
        incident.message = oss.str();
        return incident;
    }

    const auto projected_crossing_time_ms =
        std::chrono::duration<double, std::milli>(
            std::chrono::duration<double>(remaining_angle_deg / angle_growth_rate_deg_per_sec)).count();

    if (projected_crossing_time_ms > config_.projected_angle_limit_horizon.count())
    {
        return std::nullopt;
    }

    Incident incident;
    incident.code = IncidentCode::AngleGrowthTooFast;
    incident.source_id = source_id;
    incident.last_angle_deg = state.last_sample->angle_deg;
    incident.angle_growth_rate_deg_per_sec = angle_growth_rate_deg_per_sec;
    incident.projected_limit_crossing_time_ms = projected_crossing_time_ms;
    std::ostringstream oss;
    oss.setf(std::ios::fixed);
    oss.precision(2);
    oss << "Angle source '" << source_id.value
        << "' is growing too fast: angle " << state.last_sample->angle_deg
        << "°, rate " << angle_growth_rate_deg_per_sec << "°/s, projected 340° crossing in ";
    oss.precision(0);
    oss << projected_crossing_time_ms << " ms";
    incident.message = oss.str();

    return incident;
}

} // namespace application::orchestrators
