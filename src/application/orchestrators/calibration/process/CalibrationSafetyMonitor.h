#ifndef CLEANGRADUATOR_CALIBRATIONSAFETYMONITOR_H
#define CLEANGRADUATOR_CALIBRATIONSAFETYMONITOR_H

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "domain/core/angle/AngleSourceEvent.h"
#include "domain/core/angle/AngleSourcePacket.h"
#include "shared/watchdog/SoftwareWatchdog.h"

namespace application::orchestrators {

struct CalibrationSafetyIncident {
    enum class Code {
        AngleSourceTimedOut,
        PredictedAngleOverflow,
    };

    Code code;
    std::string message;
    std::optional<domain::common::SourceId> source_id;
    std::optional<double> current_angle_deg;
    std::optional<double> predicted_angle_deg;
    std::optional<double> angular_speed_deg_per_sec;
};

class CalibrationSafetyMonitor final {
public:
    using FaultHandler = std::function<void(const CalibrationSafetyIncident&)>;

    struct Config {
        std::chrono::milliseconds angle_watchdog_timeout{1500};
        std::chrono::milliseconds polling_period{50};
        double max_safe_angle_deg{340.0};
    };

    CalibrationSafetyMonitor();
    explicit CalibrationSafetyMonitor(Config config);
    ~CalibrationSafetyMonitor();

    void start(const std::set<domain::common::SourceId>& opened_angle_sources, FaultHandler handler);
    void stop();

    void onAnglePacket(const domain::common::AngleSourcePacket& packet);
    void onAngleSourceEvent(const domain::common::AngleSourceEvent& event);

private:
    struct AngleSampleSnapshot {
        domain::common::Timestamp timestamp;
        double angle_deg{0.0};
    };

    class ICalibrationSafetyRule {
    public:
        virtual ~ICalibrationSafetyRule() = default;
        virtual void onStart(const std::set<domain::common::SourceId>& opened_angle_sources) = 0;
        virtual void onStop() = 0;
        virtual std::optional<CalibrationSafetyIncident> onAnglePacket(const domain::common::AngleSourcePacket& packet) = 0;
        virtual std::optional<CalibrationSafetyIncident> onAngleSourceEvent(const domain::common::AngleSourceEvent& event) = 0;
        virtual std::optional<CalibrationSafetyIncident> poll() = 0;
    };

    class AngleSourceWatchdogRule;
    class AngleGrowthSafetyRule;

    void workerLoop();
    void reportIncident(const CalibrationSafetyIncident& incident);

private:
    Config config_;
    std::vector<std::unique_ptr<ICalibrationSafetyRule>> rules_;

    std::mutex mutex_;
    FaultHandler fault_handler_;
    std::thread worker_;
    std::atomic<bool> running_{false};
    std::atomic<bool> incident_reported_{false};
};

} // namespace application::orchestrators

#endif // CLEANGRADUATOR_CALIBRATIONSAFETYMONITOR_H
