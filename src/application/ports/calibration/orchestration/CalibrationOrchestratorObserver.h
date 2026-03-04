#ifndef CLEANGRADUATOR_CALIBRATIONORCHESTRATOROBSERVER_H
#define CLEANGRADUATOR_CALIBRATIONORCHESTRATOROBSERVER_H

namespace application::orchestrators {
    class CalibrationOrchestratorEvent;
}

namespace application::ports {
    struct CalibrationOrchestratorObserver {
        virtual ~CalibrationOrchestratorObserver() = default;
        virtual void onCalibrationOrchestratorEvent(const orchestrators::CalibrationOrchestratorEvent& ev) = 0;
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONORCHESTRATOROBSERVER_H