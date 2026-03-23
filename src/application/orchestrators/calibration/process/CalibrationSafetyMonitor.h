#ifndef CLEANGRADUATOR_CALIBRATIONSAFETYMONITOR_H
#define CLEANGRADUATOR_CALIBRATIONSAFETYMONITOR_H

#include <chrono>
#include <mutex>
#include <optional>
#include <string>
#include <set>
#include <unordered_map>

#include "domain/core/angle/AngleSourcePacket.h"
#include "domain/core/angle/SourceId.h"
#include "shared/watchdog/SoftwareWatchdog.h"

namespace application::orchestrators {

class CalibrationSafetyMonitor final {
public:
    struct Config {
        std::chrono::milliseconds angle_source_silence_timeout{1500};
        double max_safe_angle_deg{340.0};
        double max_safe_angle_growth_rate_deg_per_sec{45.0};
        std::chrono::milliseconds projected_angle_limit_horizon{1500};
    };

    enum class IncidentCode {
        AngleSourceSilence,
        AngleGrowthTooFast
    };

    struct Incident {
        IncidentCode code;
        std::string message;
        std::optional<domain::common::SourceId> source_id;
        std::optional<double> last_angle_deg;
        std::optional<double> angle_growth_rate_deg_per_sec;
        std::optional<double> projected_limit_crossing_time_ms;
    };

    CalibrationSafetyMonitor();
    explicit CalibrationSafetyMonitor(Config config);
    ~CalibrationSafetyMonitor();

    void start(const std::set<domain::common::SourceId>& source_ids);
    void stop();

    void onAnglePacket(const domain::common::AngleSourcePacket& packet);
    std::optional<Incident> detectIncident() const;

private:
    struct AngleSample {
        domain::common::Timestamp timestamp;
        double angle_deg{0.0};
    };

    struct SourceState {
        shared::watchdog::SoftwareWatchdog watchdog;
        std::optional<AngleSample> previous_sample;
        std::optional<AngleSample> last_sample;
    };

    std::optional<Incident> detectSilenceIncidentLocked() const;
    std::optional<Incident> detectAngleGrowthIncidentLocked(
        const domain::common::SourceId& source_id,
        const SourceState& state) const;

private:
    Config config_;
    mutable std::mutex mutex_;
    std::unordered_map<int, SourceState> source_states_;
};

} // namespace application::orchestrators

#endif // CLEANGRADUATOR_CALIBRATIONSAFETYMONITOR_H
