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
        struct StatusText {
            std::string text;
        };
        struct StartFailed {
            std::string error;
        };

        using Data = std::variant<Started, Stopped, Failed, StatusText>;
        Data data;

        CalibrationOrchestratorEvent(Data data) : data(std::move(data)) {};

    };
}

#endif //CLEANGRADUATOR_CALIBRATIONORCHESTRATOREVENT_H
