#ifndef CLEANGRADUATOR_CALIBRATIONORCHESTRATORSTATE_H
#define CLEANGRADUATOR_CALIBRATIONORCHESTRATORSTATE_H

namespace application::orchestrators {
    enum class CalibrationOrchestratorState {
        Stopped,
        Starting,
        Started,
        Stopping
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONORCHESTRATORSTATE_H