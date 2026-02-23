#ifndef CLEANGRADUATOR_CALIBRATIONSESSIONCONTROLLER_H
#define CLEANGRADUATOR_CALIBRATIONSESSIONCONTROLLER_H
#include "CalibrationSessionControllerInput.h"
#include "CalibrationSessionControllerPorts.h"
#include "application/orchestrators/calibration/process/CalibrationProcessOrchestrator.h"
#include "application/orchestrators/calibration/process/CalibrationProcessOrchestratorInput.h"
#include "application/orchestrators/settings/CalibrationSettingsQuery.h"

namespace application::orchestrators {
    class CalibrationSessionController {
    public:
        explicit CalibrationSessionController(CalibrationSessionControllerPorts ports, CalibrationProcessOrchestrator& runtime);
        ~CalibrationSessionController();

        bool isRunning() const;

        void start(CalibrationSessionControllerInput inp);
        void stop();
        void abort();

    private:
        std::optional<CalibrationProcessOrchestratorInput> buildConfig();

    private:
        CalibrationProcessOrchestrator& runtime_;
        domain::common::CalibrationMode calibration_mode_{domain::common::CalibrationMode::Full};

        fmt::Logger logger_;
        CalibrationSettingsQuery& settings_query_;
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONSESSIONCONTROLLER_H