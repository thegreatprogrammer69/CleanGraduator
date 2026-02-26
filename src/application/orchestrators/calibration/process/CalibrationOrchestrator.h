#ifndef CLEANGRADUATOR_CALIBRATIONPROCESSORCHESTRATOR_H
#define CLEANGRADUATOR_CALIBRATIONPROCESSORCHESTRATOR_H

#include <atomic>
#include <optional>

#include "CalibrationOrchestratorInput.h"
#include "CalibrationOrchestratorPorts.h"
#include "domain/core/calibration/lifecycle/CalibrationLifecycleState.h"
#include "domain/ports/angle/IAngleSink.h"
#include "domain/ports/angle/IAngleSourceObserver.h"
#include "domain/ports/drivers/motor/IMotorDriverObserver.h"
#include "domain/ports/pressure/IPressureSink.h"
#include "domain/ports/pressure/IPressureSourceObserver.h"

namespace application::orchestrators {

class CalibrationOrchestrator final
    : domain::ports::IPressureSourceObserver
    , domain::ports::IPressureSink
    , domain::ports::IAngleSourceObserver
    , domain::ports::IAngleSink
    , domain::ports::IMotorDriverObserver
{
public:
    explicit CalibrationOrchestrator(CalibrationOrchestratorPorts ports);
    ~CalibrationOrchestrator() override;

    // API
    void start(CalibrationOrchestratorInput input);
    void stop();
    bool isRunning() const;

private:
    using LifecycleState = domain::common::CalibrationLifecycleState;

    LifecycleState lifecycleState() const;

    void attachObservers()
    void detachObservers();

private:
    // Config
    CalibrationOrchestratorInput config_{};

    // Trackers/state
    domain::common::PressurePointsTracker pressure_points_tracker_;

    std::atomic<bool> homing_stop_requested_{false};

    std::atomic<bool> sources_attached_{false};
    std::atomic<bool> actuators_attached_{false};

    std::atomic<bool> session_begun_{false};
    std::atomic<bool> tracking_begun_{false};

    std::atomic<int> current_point_index_{-1};

    // Observe-only motor facts (no motor control calls)
    std::atomic<bool> motor_running_{false};
    std::atomic<domain::common::MotorDirection> last_direction_{domain::common::MotorDirection::Forward};
    std::atomic<domain::common::MotorLimitsState> last_limits_{};

    // Ports
    fmt::Logger logger_;
    domain::ports::IPressureSource& pressure_source_;
    ports::IVideoAngleSourcesStorage& angle_sources_storage_;
    domain::ports::IMotorDriver& motor_driver_;   // observe-only usage
    domain::ports::IValveDriver& valve_driver_;   // observe-only usage
    domain::ports::ICalibrationStrategy& strategy_;
    domain::ports::ICalibrationRecorder& recorder_;
    domain::ports::ICalibrationLifecycle& lifecycle_;
};

} // namespace application::orchestrators

#endif // CLEANGRADUATOR_CALIBRATIONPROCESSORCHESTRATOR_H