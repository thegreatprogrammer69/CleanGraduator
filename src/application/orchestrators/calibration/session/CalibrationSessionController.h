#ifndef CLEANGRADUATOR_CALIBRATIONSESSIONCONTROLLER_H
#define CLEANGRADUATOR_CALIBRATIONSESSIONCONTROLLER_H
#include "CalibrationSessionControllerInput.h"
#include "CalibrationSessionControllerPorts.h"
#include "application/orchestrators/calibration/process/CalibrationProcessOrchestrator.h"
#include "application/orchestrators/calibration/process/CalibrationProcessOrchestratorInput.h"
#include "application/orchestrators/settings/CalibrationSettingsQuery.h"
#include "domain/ports/calibration/lifecycle/ICalibrationLifecycle.h"

namespace application::orchestrators {
    class CalibrationSessionController : public domain::ports::ICalibrationLifecycle {
    public:
        explicit CalibrationSessionController(CalibrationSessionControllerPorts ports, CalibrationProcessOrchestrator& runtime);
        ~CalibrationSessionController();

        bool isRunning() const;


        bool start() override;
        void markRunning() override;

        void markIdle() override;

        void markError(const std::string& err) override;

        std::string lastError() const override;

        domain::common::CalibrationLifecycleState state() const override;

        void addObserver(domain::ports::ICalibrationLifecycleObserver& observer) override;
        void removeObserver(domain::ports::ICalibrationLifecycleObserver& observer) override;
        void start(CalibrationSessionControllerInput inp);
        bool stop() override;
        void abort();

    private:
        std::optional<CalibrationProcessOrchestratorInput> buildConfig();

    private:
        CalibrationProcessOrchestrator& runtime_;
        domain::common::CalibrationMode calibration_mode_{domain::common::CalibrationMode::Full};

        fmt::Logger logger_;
        CalibrationSettingsQuery& settings_query_;
        domain::ports::ICalibrationLifecycle& lifecycle_;
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONSESSIONCONTROLLER_H