#ifndef CLEANGRADUATOR_CALIBRATIONSAFETYMONITOR_H
#define CLEANGRADUATOR_CALIBRATIONSAFETYMONITOR_H

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "domain/core/angle/AngleSourcePacket.h"
#include "domain/core/angle/SourceId.h"
#include "domain/core/pressure/PressurePacket.h"
#include "domain/fmt/Logger.h"

namespace application::orchestrators {

class CalibrationSafetyMonitor final {
public:
    struct Incident {
        std::string code;
        std::string message;
        std::vector<std::pair<std::string, std::string>> details;
    };

    explicit CalibrationSafetyMonitor(fmt::Logger logger);
    ~CalibrationSafetyMonitor();

    void start(const std::vector<domain::common::SourceId>& opened_angle_sources);
    void stop();

    std::optional<Incident> onPressurePacket(const domain::common::PressurePacket& packet);
    std::optional<Incident> onAnglePacket(const domain::common::AngleSourcePacket& packet);

public:
    class Rule;

private:
    std::optional<Incident> processIncident(const std::optional<Incident>& incident);
    std::optional<Incident> pollRules();

private:
    fmt::Logger logger_;
    std::vector<std::unique_ptr<Rule>> rules_;
    bool running_{false};
};

} // namespace application::orchestrators

#endif // CLEANGRADUATOR_CALIBRATIONSAFETYMONITOR_H
