#ifndef CLEANGRADUATOR_CALIBRATIONSESSIONCONTROL_H
#define CLEANGRADUATOR_CALIBRATIONSESSIONCONTROL_H

#include <optional>
#include <string>

#include "application/models/info/CalibrationContext.h"
#include "application/orchestrators/calibration/process/CalibrationOrchestrator.h"
#include "application/orchestrators/settings/CalibrationContextProvider.h"
#include "domain/core/calibration/common/CalibrationMode.h"

namespace application::usecase {

    class CalibrationSessionControl {
    public:
        CalibrationSessionControl(
            orchestrators::CalibrationOrchestrator& orchestrator,
            orchestrators::CalibrationContextProvider& context_provider)
            : orchestrator_(orchestrator)
            , context_provider_(context_provider) {}

        bool isRunning() const;

        bool start(
            domain::common::CalibrationMode mode,
            std::string& error_text);

        void stop();
        void emergencyStop();

        void addObserver(ports::CalibrationOrchestratorObserver& observer);
        void removeObserver(ports::CalibrationOrchestratorObserver& observer);

    private:
        std::optional<models::CalibrationContext> loadCalibrationContext(
            std::string& error_text);

        orchestrators::CalibrationOrchestrator& orchestrator_;
        orchestrators::CalibrationContextProvider& context_provider_;
    };

}

#endif // CLEANGRADUATOR_CALIBRATIONSESSIONCONTROL_H
