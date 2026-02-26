#ifndef CLEANGRADUATOR_CALIBRATIONSESSIONCONTROLLER_H
#define CLEANGRADUATOR_CALIBRATIONSESSIONCONTROLLER_H
#include "CalibrationSessionControllerInput.h"
#include "CalibrationSessionControllerPorts.h"
#include "application/orchestrators/calibration/process/CalibrationOrchestrator.h"
#include "application/orchestrators/calibration/process/CalibrationOrchestratorInput.h"
#include "application/orchestrators/settings/CalibrationSettingsQuery.h"

namespace application::orchestrators {
    class CalibrationSessionController {
    public:
        explicit CalibrationSessionController(CalibrationSessionControllerPorts ports, CalibrationOrchestrator& runtime);
        ~CalibrationSessionController();

        bool isRunning() const;

        void start(CalibrationSessionControllerInput inp);
        void stop();
        void abort();

    private:
        std::optional<CalibrationOrchestratorInput> buildConfig();

    private:
        CalibrationOrchestrator& runtime_;
        domain::common::CalibrationMode calibration_mode_{domain::common::CalibrationMode::Full};

        fmt::Logger logger_;
        CalibrationSettingsQuery& settings_query_;
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONSESSIONCONTROLLER_H