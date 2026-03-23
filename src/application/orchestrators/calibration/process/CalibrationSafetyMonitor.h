#ifndef CLEANGRADUATOR_CALIBRATIONSAFETYMONITOR_H
#define CLEANGRADUATOR_CALIBRATIONSAFETYMONITOR_H

#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "domain/core/angle/AngleSourcePacket.h"
#include "domain/core/angle/SourceId.h"
#include "domain/core/measurement/Angle.h"
#include "domain/core/measurement/AngleUnit.h"
#include "domain/core/measurement/Timestamp.h"
#include "domain/fmt/Logger.h"
#include "shared/watchdog/SoftwareWatchdog.h"

namespace application::orchestrators {

class CalibrationSafetyMonitor final {
public:
    struct Failure {
        enum class Code {
            AngleStreamTimeout,
            AngleGrowthTooFast,
        };

        Code code;
        std::string message;
        std::optional<domain::common::SourceId> source_id;
        std::optional<double> current_angle_deg;
        std::optional<double> projected_angle_deg;
        std::optional<double> angular_speed_deg_per_sec;
    };

    using FailureHandler = std::function<void(const Failure&)>;

    struct Config {
        std::chrono::milliseconds angle_watchdog_timeout{1500};
        std::chrono::milliseconds watchdog_poll_period{100};
        domain::common::Angle max_safe_angle = domain::common::Angle::fromDegrees(340.0);
        std::chrono::seconds max_projection_horizon{2};
    };

    explicit CalibrationSafetyMonitor(fmt::Logger logger);
    CalibrationSafetyMonitor(fmt::Logger logger, Config config);
    ~CalibrationSafetyMonitor();

    void start(const std::vector<domain::common::SourceId>& opened_sources, FailureHandler on_failure);
    void stop();

    void onAnglePacket(const domain::common::AngleSourcePacket& packet);

private:
    struct AngleTrackState {
        shared::watchdog::SoftwareWatchdog watchdog;
        std::optional<domain::common::AngleSourcePacket> previous_packet;
        std::optional<domain::common::AngleSourcePacket> current_packet;
    };

private:
    void watchdogLoop();
    void reportFailure(Failure failure);
    std::optional<Failure> checkForAngleProjectionFailure(
        const domain::common::AngleSourcePacket& previous,
        const domain::common::AngleSourcePacket& current) const;
    static const char* toString(Failure::Code code);

private:
    fmt::Logger logger_;
    Config config_;

    std::mutex mutex_;
    std::unordered_map<domain::common::SourceId, AngleTrackState> angle_states_;
    FailureHandler on_failure_;
    std::optional<Failure> failure_;

    std::atomic<bool> running_{false};
    std::thread watchdog_thread_;
};

} // namespace application::orchestrators

#endif // CLEANGRADUATOR_CALIBRATIONSAFETYMONITOR_H
