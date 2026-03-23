#ifndef CLEANGRADUATOR_CALIBRATIONPROCESSORCHESTRATOR_H
#define CLEANGRADUATOR_CALIBRATIONPROCESSORCHESTRATOR_H

#include <atomic>
#include <functional>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "CalibrationLayoutBuilder.h"
#include "CalibrationOrchestratorInput.h"
#include "CalibrationOrchestratorPorts.h"
#include "CalibrationOrchestratorState.h"
#include "CalibrationStartError.h"
#include "application/common/OperationResult.h"
#include "application/orchestrators/calibration/process/CalibrationOrchestratorEvent.h"
#include "application/ports/calibration/orchestration/CalibrationOrchestratorObserver.h"
#include "domain/core/angle/SourceId.h"
#include "domain/core/calibration/strategy/CalibrationStrategyVerdict.h"
#include "domain/fmt/Logger.h"
#include "domain/ports/angle/IAngleSink.h"
#include "domain/ports/angle/IAngleSourceObserver.h"
#include "domain/ports/drivers/motor/IMotorDriverObserver.h"
#include "domain/ports/pressure/IPressureSink.h"
#include "domain/ports/pressure/IPressureSourceObserver.h"
#include "../../../../shared/list/ThreadSafeObserverList.h"

namespace application::ports {
    struct CalibrationOrchestratorObserver;
}

namespace application::orchestrators {

class CalibrationOrchestrator final
    : public domain::ports::IPressureSourceObserver
    , public domain::ports::IPressureSink
    , public domain::ports::IAngleSourceObserver
    , public domain::ports::IAngleSink
    , public domain::ports::IMotorDriverObserver
{
public:
    explicit CalibrationOrchestrator(CalibrationOrchestratorPorts ports);
    ~CalibrationOrchestrator() override;

    bool start(CalibrationOrchestratorInput input);
    void stop();
    bool isRunning() const;

    void addObserver(ports::CalibrationOrchestratorObserver& observer);
    void removeObserver(ports::CalibrationOrchestratorObserver& observer);

    void onPressureSourceEvent(const domain::common::PressureSourceEvent&) override;
    void onPressurePacket(const domain::common::PressurePacket&) override;
    void onAnglePacket(const domain::common::AngleSourcePacket&) override;
    void onAngleSourceEvent(const domain::common::AngleSourceEvent&) override;
    void onMotorEvent(const domain::common::MotorDriverEvent& event) override;

private:
    using StrategyVerdict = domain::common::CalibrationStrategyVerdict;

    struct StrategyExecutionResult
    {
        bool complete{false};
        std::optional<std::string> fault;
    };

    using StartResult = application::common::OperationResult<CalibrationStartError>;
    using CompensationAction = std::function<void()>;
    using CompensationStack = std::vector<CompensationAction>;

private:
    void attachObservers();
    void detachObservers();

    void notifyObservers(const CalibrationOrchestratorEvent& ev);

    void teardown();
    void stopWithError(const std::string& error);

    StrategyExecutionResult applyVerdict(const StrategyVerdict& verdict);

    void applyCommand(const StrategyVerdict::BeginSession& cmd);
    void applyCommand(const StrategyVerdict::EndSession& cmd);
    void applyCommand(const StrategyVerdict::MotorSetFrequency& cmd);
    void applyCommand(const StrategyVerdict::MotorSetDirection& cmd);
    void applyCommand(const StrategyVerdict::MotorSetFlaps& cmd);
    void applyCommand(const StrategyVerdict::MotorStart& cmd);
    void applyCommand(const StrategyVerdict::MotorStop& cmd);

    StartResult prepareMotor(CompensationStack& rollback);
    StartResult prepareSources(CompensationStack& rollback);
    StartResult attachObserversPhase(CompensationStack& rollback);
    StartResult startPressureSource(CompensationStack& rollback);
    StartResult beginStrategy(CompensationStack& rollback);
    StartResult startRecording(CompensationStack& rollback);
    StartResult publishStarted();

    void rollbackStart(CompensationStack& rollback) noexcept;

    static void pushRollback(CompensationStack& rollback, CompensationAction action);
    static StartResult makeStartFailure(std::string message);

private:
    std::atomic<CalibrationOrchestratorState> state_{
        CalibrationOrchestratorState::Stopped
    };

    std::set<domain::common::SourceId> opened_angle_sources_;

    mutable std::mutex lifecycle_mutex_;
    ThreadSafeObserverList<ports::CalibrationOrchestratorObserver> observers_;

    fmt::Logger logger_;

    CalibrationOrchestratorPorts ports_;
    CalibrationOrchestratorInput inp_;
    CalibrationLayoutBuilder layout_builder_;
};

} // namespace application::orchestrators

#endif // CLEANGRADUATOR_CALIBRATIONPROCESSORCHESTRATOR_H
