#include "CalibrationSafetyMonitor.h"

#include <chrono>
#include <cmath>
#include <optional>
#include <sstream>
#include <unordered_map>
#include <utility>

#include "shared/watchdog/SoftwareWatchdog.h"

namespace application::orchestrators {

class CalibrationSafetyMonitor::Rule {
public:
    virtual ~Rule() = default;

    virtual void start(const std::vector<domain::common::SourceId>&) {}
    virtual void stop() {}

    virtual std::optional<CalibrationSafetyMonitor::Incident> onPressurePacket(const domain::common::PressurePacket&)
    {
        return std::nullopt;
    }

    virtual std::optional<CalibrationSafetyMonitor::Incident> onAnglePacket(const domain::common::AngleSourcePacket&)
    {
        return std::nullopt;
    }

    virtual std::optional<CalibrationSafetyMonitor::Incident> poll()
    {
        return std::nullopt;
    }
};

namespace {

using domain::common::AngleSourcePacket;
using domain::common::PressurePacket;
using domain::common::SourceId;
using Incident = CalibrationSafetyMonitor::Incident;

constexpr auto kAnglePacketTimeout = std::chrono::milliseconds(1500);
constexpr double kCriticalAngleDeg = 340.0;
constexpr double kProjectionHorizonSec = 1.5;

std::string toString(const double value)
{
    std::ostringstream ss;
    ss << value;
    return ss.str();
}

std::string describeIncident(const Incident& incident)
{
    std::ostringstream ss;
    ss << incident.code << ": " << incident.message;

    if (!incident.details.empty())
    {
        ss << " [";
        bool first = true;
        for (const auto& [key, value] : incident.details)
        {
            if (!first)
                ss << ", ";
            first = false;
            ss << key << '=' << value;
        }
        ss << ']';
    }

    return ss.str();
}

class AngleSourceTimeoutRule final : public CalibrationSafetyMonitor::Rule {
public:
    void start(const std::vector<SourceId>& opened_angle_sources) override
    {
        watchdogs_.clear();
        watchdogs_.reserve(opened_angle_sources.size());

        for (const auto& source_id : opened_angle_sources)
        {
            auto& state = watchdogs_[source_id.value];
            state.source_id = source_id;
            state.watchdog.start(kAnglePacketTimeout);
        }
    }

    void stop() override
    {
        for (auto& [_, state] : watchdogs_)
            state.watchdog.stop();

        watchdogs_.clear();
    }

    std::optional<Incident> onAnglePacket(const AngleSourcePacket& packet) override
    {
        if (const auto it = watchdogs_.find(packet.source_id.value); it != watchdogs_.end())
            it->second.watchdog.feed();

        return poll();
    }

    std::optional<Incident> onPressurePacket(const PressurePacket&) override
    {
        return poll();
    }

    std::optional<Incident> poll() override
    {
        for (auto& [_, state] : watchdogs_)
        {
            if (!state.watchdog.expired())
                continue;

            state.watchdog.stop();

            Incident incident;
            incident.code = "angle_source_timeout";
            incident.message = "Angle packets are absent for more than 1500 ms";
            incident.details.emplace_back("source_id", std::to_string(state.source_id.value));
            incident.details.emplace_back("timeout_ms", std::to_string(kAnglePacketTimeout.count()));
            return incident;
        }

        return std::nullopt;
    }

private:
    struct WatchdogState {
        SourceId source_id{};
        shared::watchdog::SoftwareWatchdog watchdog;
    };

    std::unordered_map<int, WatchdogState> watchdogs_;
};

class AngleGrowthHazardRule final : public CalibrationSafetyMonitor::Rule {
public:
    std::optional<Incident> onAnglePacket(const AngleSourcePacket& packet) override
    {
        auto& state = states_[packet.source_id.value];
        const double timestamp_sec = packet.timestamp.asSeconds();
        const double angle_deg = packet.angle.degrees();

        if (state.has_last)
        {
            const double dt = timestamp_sec - state.last_timestamp_sec;
            if (dt > 0.0)
            {
                const double speed_deg_per_sec = (angle_deg - state.last_angle_deg) / dt;
                if (speed_deg_per_sec > 0.0)
                {
                    const double projected_angle = angle_deg + speed_deg_per_sec * kProjectionHorizonSec;
                    if (projected_angle >= kCriticalAngleDeg)
                    {
                        Incident incident;
                        incident.code = "angle_growth_hazard";
                        incident.message = "Angle grows too fast and may soon exceed 340 degrees; likely wrong gauge range selected";
                        incident.details.emplace_back("source_id", std::to_string(packet.source_id.value));
                        incident.details.emplace_back("angle_deg", toString(angle_deg));
                        incident.details.emplace_back("speed_deg_per_sec", toString(speed_deg_per_sec));
                        incident.details.emplace_back("projected_angle_deg", toString(projected_angle));
                        incident.details.emplace_back("projection_horizon_sec", toString(kProjectionHorizonSec));
                        return incident;
                    }
                }
            }
        }

        state.last_timestamp_sec = timestamp_sec;
        state.last_angle_deg = angle_deg;
        state.has_last = true;

        return std::nullopt;
    }

    void stop() override
    {
        states_.clear();
    }

private:
    struct SourceState {
        double last_timestamp_sec{0.0};
        double last_angle_deg{0.0};
        bool has_last{false};
    };

    std::unordered_map<int, SourceState> states_;
};

} // namespace


CalibrationSafetyMonitor::CalibrationSafetyMonitor(fmt::Logger logger)
    : logger_(std::move(logger))
{
    rules_.emplace_back(std::make_unique<AngleSourceTimeoutRule>());
    rules_.emplace_back(std::make_unique<AngleGrowthHazardRule>());
}

CalibrationSafetyMonitor::~CalibrationSafetyMonitor()
{
    stop();
}

void CalibrationSafetyMonitor::start(const std::vector<SourceId>& opened_angle_sources)
{
    stop();

    for (auto& rule : rules_)
        rule->start(opened_angle_sources);

    running_ = true;
}

void CalibrationSafetyMonitor::stop()
{
    if (!running_)
        return;

    for (auto& rule : rules_)
        rule->stop();

    running_ = false;
}

std::optional<Incident> CalibrationSafetyMonitor::onPressurePacket(const PressurePacket& packet)
{
    if (!running_)
        return std::nullopt;

    for (auto& rule : rules_)
    {
        if (const auto incident = processIncident(rule->onPressurePacket(packet)))
            return incident;
    }

    return pollRules();
}

std::optional<Incident> CalibrationSafetyMonitor::onAnglePacket(const AngleSourcePacket& packet)
{
    if (!running_)
        return std::nullopt;

    for (auto& rule : rules_)
    {
        if (const auto incident = processIncident(rule->onAnglePacket(packet)))
            return incident;
    }

    return pollRules();
}

std::optional<Incident> CalibrationSafetyMonitor::processIncident(const std::optional<Incident>& incident)
{
    if (!incident)
        return std::nullopt;

    logger_.error("Calibration safety incident: {}", describeIncident(*incident));
    return incident;
}

std::optional<Incident> CalibrationSafetyMonitor::pollRules()
{
    for (auto& rule : rules_)
    {
        if (const auto incident = processIncident(rule->poll()))
            return incident;
    }

    return std::nullopt;
}

} // namespace application::orchestrators
