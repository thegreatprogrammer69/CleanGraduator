#include "CalibrationSafetyMonitor.h"

#include <chrono>
#include <cmath>
#include <deque>
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

    virtual void setAngleTimeoutLowPressureThreshold(const domain::common::Pressure&) {}
    virtual void onMotorDirectionChanged(domain::common::MotorDirection) {}
};

namespace {

using domain::common::AngleSourcePacket;
using domain::common::PressurePacket;
using domain::common::SourceId;
using Incident = CalibrationSafetyMonitor::Incident;

constexpr auto kAnglePacketTimeout = std::chrono::milliseconds(1500);
constexpr double kCriticalAngleDeg = 340.0;
constexpr double kProjectionHorizonSec = 0.5;

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

    std::optional<Incident> poll() override
    {
        if (!monitoring_enabled_)
            return std::nullopt;

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

    void setAngleTimeoutLowPressureThreshold(const domain::common::Pressure& threshold) override
    {
        low_pressure_threshold_pa_ = threshold.pa();
        updateMonitoringState();
    }

    void onMotorDirectionChanged(domain::common::MotorDirection direction) override
    {
        motor_direction_ = direction;
        updateMonitoringState();
    }

    std::optional<Incident> onPressurePacket(const PressurePacket& packet) override
    {
        last_pressure_pa_ = packet.pressure.pa();
        updateMonitoringState();
        return poll();
    }

private:
    void updateMonitoringState()
    {
        const bool backward = motor_direction_ == domain::common::MotorDirection::Backward;
        const bool below_half_scale =
            low_pressure_threshold_pa_ > 0.0 && last_pressure_pa_ < low_pressure_threshold_pa_;
        monitoring_enabled_ = !backward && !below_half_scale;
    }

    struct WatchdogState {
        SourceId source_id{};
        shared::watchdog::SoftwareWatchdog watchdog;
    };

    std::unordered_map<int, WatchdogState> watchdogs_;
    domain::common::MotorDirection motor_direction_{domain::common::MotorDirection::Forward};
    double low_pressure_threshold_pa_{0.0};
    double last_pressure_pa_{0.0};
    bool monitoring_enabled_{true};
};

class AngleGrowthHazardRule final : public CalibrationSafetyMonitor::Rule {
public:
    std::optional<Incident> onAnglePacket(const AngleSourcePacket& packet) override
    {
        const int sourceId = packet.source_id.value;
        auto& state = states_[sourceId];

        const double nowSec = packet.timestamp.asSeconds();
        const double nowAngleDeg = packet.angle.degrees();

        // Добавляем текущую точку
        state.points.push_back(Point{nowSec, nowAngleDeg});

        // Удаляем всё, что старее 1 секунды относительно текущей точки,
        // но оставляем хотя бы одну точку до границы окна, чтобы можно было
        // взять точку "примерно 1 секунду назад".
        while (state.points.size() >= 2 &&
               state.points[1].timestampSec <= nowSec - kReferenceDeltaSec)
        {
            state.points.pop_front();
        }

        // Нужна точка, которая находится не позже чем на 1 секунду назад.
        // После цикла это state.points.front():
        // - либо ближайшая точка <= nowSec - 1.0
        // - либо, если поток слишком редкий, просто самая старая доступная точка
        if (state.points.size() >= 2) {
            const Point& refPoint = state.points.front();

            const double dt = nowSec - refPoint.timestampSec;
            if (dt >= kMinDtSec) {
                const double speedDegPerSec = (nowAngleDeg - refPoint.angleDeg) / dt;

                if (speedDegPerSec > 0.0) {
                    const double projectedAngleDeg =
                        nowAngleDeg + speedDegPerSec * kProjectionHorizonSec;

                    if (projectedAngleDeg >= kCriticalAngleDeg) {
                        Incident incident;
                        incident.code = "angle_growth_hazard";
                        incident.message =
                            "Angle grows too fast and may soon exceed 340 degrees; "
                            "likely wrong gauge range selected";

                        incident.details.emplace_back("source_id", std::to_string(sourceId));
                        incident.details.emplace_back("angle_deg", toString(nowAngleDeg));
                        incident.details.emplace_back("reference_angle_deg", toString(refPoint.angleDeg));
                        incident.details.emplace_back("dt_sec", toString(dt));
                        incident.details.emplace_back("speed_deg_per_sec", toString(speedDegPerSec));
                        incident.details.emplace_back("projected_angle_deg", toString(projectedAngleDeg));
                        incident.details.emplace_back("projection_horizon_sec", toString(kProjectionHorizonSec));

                        return incident;
                    }
                }
            }
        }

        return std::nullopt;
    }

    void stop() override
    {
        states_.clear();
    }

private:
    struct Point {
        double timestampSec{0.0};
        double angleDeg{0.0};
    };

    struct SourceState {
        std::deque<Point> points;
    };

    static constexpr double kReferenceDeltaSec = 1.0;   // хотим брать точку ~1 секунду назад
    static constexpr double kMinDtSec = 0.95;           // защита от слишком малого dt
    static constexpr double kProjectionHorizonSec = 1.0;
    static constexpr double kCriticalAngleDeg = 340.0;

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

void CalibrationSafetyMonitor::setAngleTimeoutLowPressureThreshold(const domain::common::Pressure& threshold)
{
    for (auto& rule : rules_)
        rule->setAngleTimeoutLowPressureThreshold(threshold);
}

void CalibrationSafetyMonitor::setMotorDirection(domain::common::MotorDirection direction)
{
    for (auto& rule : rules_)
        rule->onMotorDirectionChanged(direction);
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
