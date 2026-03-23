#include "CalibrationSafetyMonitor.h"

#include <cmath>
#include <sstream>
#include <type_traits>
#include <utility>
#include <variant>

namespace application::orchestrators {
namespace {

using domain::common::AngleSourceEvent;
using domain::common::AngleSourcePacket;
using domain::common::SourceId;

std::string buildAngleWatchdogMessage(SourceId source_id, std::chrono::milliseconds timeout)
{
    std::ostringstream oss;
    oss << "Angle source " << source_id.value << " timed out: no angle packets for "
        << timeout.count() << " ms";
    return oss.str();
}

std::string buildPredictedOverflowMessage(
    SourceId source_id,
    double current_angle_deg,
    double predicted_angle_deg,
    double angular_speed_deg_per_sec)
{
    std::ostringstream oss;
    oss << "Angle source " << source_id.value
        << " indicates unsafe gauge selection: current angle " << current_angle_deg
        << " deg, predicted next angle " << predicted_angle_deg
        << " deg, angular speed " << angular_speed_deg_per_sec
        << " deg/s. Safe limit is 340 deg";
    return oss.str();
}

} // namespace

class CalibrationSafetyMonitor::AngleSourceWatchdogRule final
    : public CalibrationSafetyMonitor::ICalibrationSafetyRule {
public:
    explicit AngleSourceWatchdogRule(std::chrono::milliseconds timeout)
        : timeout_(timeout)
    {
    }

    void onStart(const std::set<SourceId>& opened_angle_sources) override
    {
        watchdogs_.clear();
        for (const auto& source_id : opened_angle_sources)
        {
            auto& watchdog = watchdogs_[source_id];
            watchdog.start(timeout_);
        }
    }

    void onStop() override
    {
        for (auto& [_, watchdog] : watchdogs_)
            watchdog.stop();
        watchdogs_.clear();
    }

    std::optional<CalibrationSafetyIncident> onAnglePacket(const AngleSourcePacket& packet) override
    {
        const auto it = watchdogs_.find(packet.source_id);
        if (it == watchdogs_.end())
            return std::nullopt;

        it->second.feed();
        return std::nullopt;
    }

    std::optional<CalibrationSafetyIncident> onAngleSourceEvent(const AngleSourceEvent&) override
    {
        return std::nullopt;
    }

    std::optional<CalibrationSafetyIncident> poll() override
    {
        for (const auto& [source_id, watchdog] : watchdogs_)
        {
            if (!watchdog.expired())
                continue;

            CalibrationSafetyIncident incident;
            incident.code = CalibrationSafetyIncident::Code::AngleSourceTimedOut;
            incident.source_id = source_id;
            incident.message = buildAngleWatchdogMessage(source_id, timeout_);
            return incident;
        }

        return std::nullopt;
    }

private:
    std::chrono::milliseconds timeout_;
    std::unordered_map<SourceId, shared::watchdog::SoftwareWatchdog> watchdogs_;
};

class CalibrationSafetyMonitor::AngleGrowthSafetyRule final
    : public CalibrationSafetyMonitor::ICalibrationSafetyRule {
public:
    explicit AngleGrowthSafetyRule(double max_safe_angle_deg)
        : max_safe_angle_deg_(max_safe_angle_deg)
    {
    }

    void onStart(const std::set<SourceId>&) override
    {
        samples_.clear();
    }

    void onStop() override
    {
        samples_.clear();
    }

    std::optional<CalibrationSafetyIncident> onAnglePacket(const AngleSourcePacket& packet) override
    {
        const auto current = AngleSampleSnapshot{packet.timestamp, packet.angle.to(domain::common::AngleUnit::deg)};
        auto& history = samples_[packet.source_id];

        if (history.has_value())
        {
            const auto dt = current.timestamp.asSeconds() - history->timestamp.asSeconds();
            const auto delta_angle_deg = current.angle_deg - history->angle_deg;

            if (dt > 0.0)
            {
                const auto angular_speed_deg_per_sec = delta_angle_deg / dt;
                if (angular_speed_deg_per_sec > 0.0)
                {
                    const auto predicted_angle_deg = current.angle_deg + angular_speed_deg_per_sec * dt;
                    if (predicted_angle_deg > max_safe_angle_deg_)
                    {
                        CalibrationSafetyIncident incident;
                        incident.code = CalibrationSafetyIncident::Code::PredictedAngleOverflow;
                        incident.source_id = packet.source_id;
                        incident.current_angle_deg = current.angle_deg;
                        incident.predicted_angle_deg = predicted_angle_deg;
                        incident.angular_speed_deg_per_sec = angular_speed_deg_per_sec;
                        incident.message = buildPredictedOverflowMessage(
                            packet.source_id,
                            current.angle_deg,
                            predicted_angle_deg,
                            angular_speed_deg_per_sec);
                        return incident;
                    }
                }
            }
        }

        history = current;
        return std::nullopt;
    }

    std::optional<CalibrationSafetyIncident> onAngleSourceEvent(const AngleSourceEvent&) override
    {
        return std::nullopt;
    }

    std::optional<CalibrationSafetyIncident> poll() override
    {
        return std::nullopt;
    }

private:
    double max_safe_angle_deg_;
    std::unordered_map<SourceId, std::optional<AngleSampleSnapshot>> samples_;
};

CalibrationSafetyMonitor::CalibrationSafetyMonitor()
    : CalibrationSafetyMonitor(Config{})
{
}

CalibrationSafetyMonitor::CalibrationSafetyMonitor(Config config)
    : config_(config)
{
    rules_.push_back(std::make_unique<AngleSourceWatchdogRule>(config_.angle_watchdog_timeout));
    rules_.push_back(std::make_unique<AngleGrowthSafetyRule>(config_.max_safe_angle_deg));
}

CalibrationSafetyMonitor::~CalibrationSafetyMonitor()
{
    stop();
}

void CalibrationSafetyMonitor::start(const std::set<SourceId>& opened_angle_sources, FaultHandler handler)
{
    stop();

    {
        std::lock_guard<std::mutex> lock(mutex_);
        fault_handler_ = std::move(handler);
        incident_reported_.store(false, std::memory_order_release);

        for (auto& rule : rules_)
            rule->onStart(opened_angle_sources);
    }

    running_.store(true, std::memory_order_release);
    worker_ = std::thread(&CalibrationSafetyMonitor::workerLoop, this);
}

void CalibrationSafetyMonitor::stop()
{
    running_.store(false, std::memory_order_release);

    if (worker_.joinable())
    {
        if (worker_.get_id() == std::this_thread::get_id())
            worker_.detach();
        else
            worker_.join();
    }

    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& rule : rules_)
        rule->onStop();
    fault_handler_ = {};
}

void CalibrationSafetyMonitor::onAnglePacket(const AngleSourcePacket& packet)
{
    std::optional<CalibrationSafetyIncident> incident_to_report;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& rule : rules_)
        {
            if (const auto incident = rule->onAnglePacket(packet))
            {
                incident_to_report = incident;
                break;
            }
        }
    }

    if (incident_to_report)
        reportIncident(*incident_to_report);
}

void CalibrationSafetyMonitor::onAngleSourceEvent(const AngleSourceEvent& event)
{
    std::optional<CalibrationSafetyIncident> incident_to_report;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& rule : rules_)
        {
            if (const auto incident = rule->onAngleSourceEvent(event))
            {
                incident_to_report = incident;
                break;
            }
        }
    }

    if (incident_to_report)
        reportIncident(*incident_to_report);
}

void CalibrationSafetyMonitor::workerLoop()
{
    while (running_.load(std::memory_order_acquire))
    {
        std::optional<CalibrationSafetyIncident> incident_to_report;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            for (auto& rule : rules_)
            {
                if (const auto incident = rule->poll())
                {
                    incident_to_report = incident;
                    break;
                }
            }
        }

        if (incident_to_report)
            reportIncident(*incident_to_report);

        std::this_thread::sleep_for(config_.polling_period);
    }
}

void CalibrationSafetyMonitor::reportIncident(const CalibrationSafetyIncident& incident)
{
    if (incident_reported_.exchange(true, std::memory_order_acq_rel))
        return;

    auto handler = fault_handler_;
    if (!handler)
        return;

    handler(incident);
}

} // namespace application::orchestrators
