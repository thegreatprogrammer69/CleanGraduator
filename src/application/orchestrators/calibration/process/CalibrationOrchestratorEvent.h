#ifndef CLEANGRADUATOR_CALIBRATIONORCHESTRATOREVENT_H
#define CLEANGRADUATOR_CALIBRATIONORCHESTRATOREVENT_H
#include <string>
#include <variant>

namespace application::orchestrators {
    struct CalibrationOrchestratorEvent {

        struct Started {};
        struct Stopped {};
        struct Failed {
            std::string error;
        };
        struct StartFailed {
            std::string error;
        };
        struct StatusChanged {
            std::string message;
        };

        using Data = std::variant<Started, Stopped, Failed, StatusChanged>;
        Data data;

        CalibrationOrchestratorEvent(Data data) : data(std::move(data)) {};

    };
}

#endif //CLEANGRADUATOR_CALIBRATIONORCHESTRATOREVENT_H
