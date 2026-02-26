#ifndef CLEANGRADUATOR_CALIBRATIONPROCESSORCHESTRATOR_H
#define CLEANGRADUATOR_CALIBRATIONPROCESSORCHESTRATOR_H

#include <atomic>
#include <optional>

#include "CalibrationProcessOrchestratorInput.h"
#include "CalibrationProcessOrchestratorPorts.h"

#include "../../../../domain/ports/calibration/tracking/IPressurePointsTrackerObserver.h"
#include "domain/core/calibration/tracking/PressurePointsTracker.h"

#include "domain/fmt/Logger.h"

#include "domain/ports/angle/IAngleSourceObserver.h"
#include "domain/ports/drivers/motor/IMotorDriverObserver.h"
#include "domain/ports/drivers/valve/IValveDriverObserver.h"
#include "domain/ports/pressure/IPressureSourceObserver.h"

namespace domain::ports {
class ICalibrationStrategy;
struct ICalibrationRecorder;
struct ICalibrationLifecycle;
} // namespace domain::ports

namespace application::orchestrators {

class CalibrationProcessOrchestrator final
    : domain::ports::IPressureSourceObserver
    , domain::ports::IAngleSourceObserver
    , domain::ports::IMotorDriverObserver
    , domain::ports::IValveDriverObserver
    , domain::common::IPressurePointsTrackerObserver
{
public:
    explicit CalibrationProcessOrchestrator(CalibrationProcessOrchestratorPorts ports);
    ~CalibrationProcessOrchestrator() override;

    // API
    void start(CalibrationProcessOrchestratorInput input);
    void stop();
    bool isRunning() const;

    void startHoming();
    void stopHoming();

    // IPressurePointsTrackerObserver
    void onPointEntered(int index) override;
    void onPointExited(int index) override;

    // IMotorDriverObserver
    void onMotorStarted() override;
    void onMotorStopped() override;
    void onMotorStartFailed(const domain::common::MotorError&) override;
    void onMotorLimitsStateChanged(domain::common::MotorLimitsState) override;
    void onMotorDirectionChanged(domain::common::MotorDirection) override;

    // IValveDriverObserver
    void onInputFlapOpened() override;
    void onOutputFlapOpened() override;
    void onFlapsClosed() override;

    // IPressureSourceObserver
    void onPressurePacket(const domain::common::PressurePacket&) override;
    void onPressureSourceOpened() override;
    void onPressureSourceOpenFailed(const domain::common::PressureSourceError&) override;
    void onPressureSourceClosed(const domain::common::PressureSourceError&) override;

    // IAngleSourceObserver
    void onAngleSourceStarted() override;
    void onAngleSourceStopped() override;
    void onAngleSourceFailed(const domain::common::AngleSourceError&) override;
    void onAngleSourcePacket(const domain::common::AngleSourcePacket&) override;

private:
    using LifecycleState = domain::common::CalibrationLifecycleState;

    // ---------------------------
    // Single-source-of-truth state
    // ---------------------------
    LifecycleState lifecycleState() const;

    // ---------------------------
    // Observers attach/detach (idempotent, split by responsibility)
    // ---------------------------
    void attachSourceObservers();    // pressure + angle sources
    void detachSourceObservers();

    void attachActuatorObservers();  // motor + valve (observe-only)
    void detachActuatorObservers();

    // ---------------------------
    // Session begin/end (idempotent)
    // ---------------------------
    void beginSession();        // called from start()
    void endSessionIfBegun();   // idempotent: strategy.end + tracker.end

    // ---------------------------
    // Internal stop/abort paths
    // ---------------------------
    void requestGracefulStop(const char* reason);
    void abortNow(const char* reason);
    void cancelStartToIdle(const char* reason);

private:
    // Config
    CalibrationProcessOrchestratorInput config_{};

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