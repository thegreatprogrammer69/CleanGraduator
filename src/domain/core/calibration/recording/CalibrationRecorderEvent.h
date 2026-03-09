#ifndef CLEANGRADUATOR_CALIBRATIONRECORDEREVENT_H
#define CLEANGRADUATOR_CALIBRATIONRECORDEREVENT_H
#include <variant>

#include "AngleSample.h"
#include "CalibrationSessionId.h"
#include "PressureSample.h"

namespace domain::common {
    struct CalibrationRecorderEvent {

        struct RecordingStarted {};
        struct RecordingStopped {};

        struct SessionStarted {
            CalibrationSessionId id;
        };

        struct PressureSampleRecorded {
            PressureSample sample;
        };

        struct AngleSampleRecorded {
            AngleSample sample;
        };

        struct SessionEnded {
            CalibrationSessionId id;
        };

        using Data = std::variant<
            RecordingStarted,
            RecordingStopped,
            SessionStarted,
            SessionEnded,
            PressureSampleRecorded,
            AngleSampleRecorded
        >;


        Data data;
        CalibrationRecorderEvent(Data data) : data(std::move(data)) {}
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONRECORDEREVENT_H