#ifndef CLEANGRADUATOR_CALIBRATIONPROCESSORCHESTRATOR_H
#define CLEANGRADUATOR_CALIBRATIONPROCESSORCHESTRATOR_H

#include <atomic>
#include <set>
#include <mutex>

#include "CalibrationOrchestratorState.h"
#include "CalibrationOrchestratorInput.h"
#include "CalibrationOrchestratorPorts.h"
#include "application/ports/calibration/orchestration/CalibrationOrchestratorObserver.h"
#include "domain/core/angle/AngleSourceId.h"
#include "domain/fmt/Logger.h"
#include "domain/ports/angle/IAngleSink.h"
#include "domain/ports/angle/IAngleSourceObserver.h"
#include "domain/ports/drivers/motor/IMotorDriverObserver.h"
#include "domain/ports/pressure/IPressureSink.h"
#include "domain/ports/pressure/IPressureSourceObserver.h"
#include "shared/ThreadSafeObserverList.h"

namespace application::ports {
    struct CalibrationOrchestratorObserver;
}

namespace application::orchestrators {

class CalibrationOrchestrator
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
    bool start(CalibrationOrchestratorInput input);
    void stop();
    bool isRunning() const;

    void addObserver(ports::CalibrationOrchestratorObserver& observer);
    void removeObserver(ports::CalibrationOrchestratorObserver& observer);

    // Observers and Sinks
    void onPressureSourceEvent(const domain::common::PressureSourceEvent &) override;
    void onPressurePacket(const domain::common::PressurePacket &) override;
    void onAnglePacket(const domain::common::AngleSourcePacket &) override;
    void onAngleSourceEvent(const domain::common::AngleSourceEvent &) override;
    void onMotorEvent(const domain::common::MotorDriverEvent &event) override;


private:
    void attachObservers();
    void detachObservers();

    void notifyObservers(const CalibrationOrchestratorEvent& ev);

    void teardown();

    void stopWithError(const std::string& error);

private:
    std::atomic<CalibrationOrchestratorState> state_;
    std::set<domain::common::AngleSourceId> opened_angle_sources_;

    mutable std::mutex lifecycle_mutex_;
    ThreadSafeObserverList<ports::CalibrationOrchestratorObserver> observers_;

    fmt::Logger logger_;

    CalibrationOrchestratorPorts ports_;
    CalibrationOrchestratorInput inp_;
};

} // namespace application::orchestrators

#endif // CLEANGRADUATOR_CALIBRATIONPROCESSORCHESTRATOR_H