#ifndef CLEANGRADUATOR_CALIBRATIONORCHESTRATOREVENT_H
#define CLEANGRADUATOR_CALIBRATIONORCHESTRATOREVENT_H
#include <string>
#include <variant>
#include "domain/core/calibration/common/CalibrationAudioCue.h"

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
        struct SoundCue {
            domain::common::CalibrationAudioCue cue;
        };
        struct StartFailed {
            std::string error;
        };

        using Data = std::variant<Started, Stopped, Failed, StatusText, SoundCue>;
        Data data;

        CalibrationOrchestratorEvent(Data data) : data(std::move(data)) {};

    };
}

#endif //CLEANGRADUATOR_CALIBRATIONORCHESTRATOREVENT_H
