#include "CalibrationSafetyMonitor.h"

#include <cmath>
#include <utility>

using namespace application::orchestrators;
using namespace domain::common;

CalibrationSafetyMonitor::CalibrationSafetyMonitor(fmt::Logger logger)
    : CalibrationSafetyMonitor(std::move(logger), Config{})
{
}

CalibrationSafetyMonitor::CalibrationSafetyMonitor(fmt::Logger logger, Config config)
    : logger_(std::move(logger))
    , config_(config)
{
}

CalibrationSafetyMonitor::~CalibrationSafetyMonitor()
{
    stop();
}

void CalibrationSafetyMonitor::start(
    const std::vector<SourceId>& opened_sources,
    FailureHandler on_failure)
{
    stop();

    {
        std::lock_guard lock(mutex_);
        on_failure_ = std::move(on_failure);
        failure_.reset();
        angle_states_.clear();

        for (const auto& source_id : opened_sources)
        {
            auto [it, inserted] = angle_states_.try_emplace(source_id);
            (void)inserted;
            it->second.watchdog.start(config_.angle_watchdog_timeout);
        }
    }

    running_.store(true, std::memory_order_release);
    watchdog_thread_ = std::thread(&CalibrationSafetyMonitor::watchdogLoop, this);
}

void CalibrationSafetyMonitor::stop()
{
    running_.store(false, std::memory_order_release);

    if (watchdog_thread_.joinable())
        watchdog_thread_.join();

    std::lock_guard lock(mutex_);
    for (auto& [source_id, state] : angle_states_)
    {
        (void)source_id;
        state.watchdog.stop();
    }
    angle_states_.clear();
    on_failure_ = {};
    failure_.reset();
}

void CalibrationSafetyMonitor::onAnglePacket(const AngleSourcePacket& packet)
{
    std::optional<Failure> detected_failure;

    {
        std::lock_guard lock(mutex_);
        auto it = angle_states_.find(packet.source_id);
        if (it == angle_states_.end())
            return;

        auto& state = it->second;
        state.watchdog.feed();

        if (state.current_packet)
            state.previous_packet = state.current_packet;

        state.current_packet = packet;

        if (state.previous_packet)
        {
            detected_failure = checkForAngleProjectionFailure(*state.previous_packet, *state.current_packet);
        }
    }

    if (detected_failure)
        reportFailure(std::move(*detected_failure));
}

void CalibrationSafetyMonitor::watchdogLoop()
{
    while (running_.load(std::memory_order_acquire))
    {
        std::this_thread::sleep_for(config_.watchdog_poll_period);

        std::optional<Failure> detected_failure;

        {
            std::lock_guard lock(mutex_);
            if (failure_.has_value())
                break;

            for (const auto& [source_id, state] : angle_states_)
            {
                if (!state.watchdog.expired())
                    continue;

                Failure failure;
                failure.code = Failure::Code::AngleStreamTimeout;
                failure.source_id = source_id;
                failure.message = fmt::format(
                    "Angle source {} timed out: no angle packets for more than {} ms",
                    source_id.value,
                    config_.angle_watchdog_timeout.count());

                if (state.current_packet)
                    failure.current_angle_deg = state.current_packet->angle.to(AngleUnit::deg);

                detected_failure = std::move(failure);
                break;
            }
        }

        if (detected_failure)
        {
            reportFailure(std::move(*detected_failure));
            break;
        }
    }
}

void CalibrationSafetyMonitor::reportFailure(Failure failure)
{
    FailureHandler handler;

    {
        std::lock_guard lock(mutex_);
        if (failure_.has_value())
            return;

        failure_ = failure;
        handler = on_failure_;
    }

    logger_.error(
        "Calibration safety failure [{}]: {}",
        toString(failure.code),
        failure.message);

    if (handler)
        handler(failure);
}

std::optional<CalibrationSafetyMonitor::Failure>
CalibrationSafetyMonitor::checkForAngleProjectionFailure(
    const AngleSourcePacket& previous,
    const AngleSourcePacket& current) const
{
    const auto delta_time = current.timestamp.toDuration() - previous.timestamp.toDuration();
    if (delta_time <= Timestamp::duration::zero())
        return std::nullopt;

    const auto delta_angle = current.angle.to(AngleUnit::deg) - previous.angle.to(AngleUnit::deg);
    const auto delta_seconds = std::chrono::duration<double>(delta_time).count();
    if (delta_seconds <= 0.0)
        return std::nullopt;

    const auto angular_speed_deg_per_sec = delta_angle / delta_seconds;
    if (!std::isfinite(angular_speed_deg_per_sec) || angular_speed_deg_per_sec <= 0.0)
        return std::nullopt;

    const auto current_angle_deg = current.angle.to(AngleUnit::deg);
    const auto remaining_deg = config_.max_safe_angle.to(AngleUnit::deg) - current_angle_deg;
    if (remaining_deg <= 0.0)
        return std::nullopt;

    const auto time_to_limit_sec = remaining_deg / angular_speed_deg_per_sec;
    if (time_to_limit_sec > std::chrono::duration<double>(config_.max_projection_horizon).count())
        return std::nullopt;

    Failure failure;
    failure.code = Failure::Code::AngleGrowthTooFast;
    failure.source_id = current.source_id;
    failure.current_angle_deg = current_angle_deg;
    failure.projected_angle_deg = config_.max_safe_angle.to(AngleUnit::deg);
    failure.angular_speed_deg_per_sec = angular_speed_deg_per_sec;
    failure.message = fmt::format(
        "Angle source {} grows too fast: {:.2f} deg/s at {:.2f} deg, projected to exceed {:.2f} deg in {:.2f} s",
        current.source_id.value,
        angular_speed_deg_per_sec,
        current_angle_deg,
        config_.max_safe_angle.to(AngleUnit::deg),
        time_to_limit_sec);

    return failure;
}

const char* CalibrationSafetyMonitor::toString(Failure::Code code)
{
    switch (code)
    {
    case Failure::Code::AngleStreamTimeout:
        return "AngleStreamTimeout";
    case Failure::Code::AngleGrowthTooFast:
        return "AngleGrowthTooFast";
    }

    return "Unknown";
}
