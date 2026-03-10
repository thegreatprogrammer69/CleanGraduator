#ifndef CLEANGRADUATOR_CALIBRATIONRECORDEREVENT_H
#define CLEANGRADUATOR_CALIBRATIONRECORDEREVENT_H
#include <variant>

#include "domain/core/calibration/common/CalibrationLayout.h"
#include "AngleSample.h"
#include "CalibrationSessionId.h"
#include "CalibrationSession.h"
#include "PressureSample.h"

namespace domain::common {
    struct CalibrationRecorderEvent {

        struct RecordingStarted {
            CalibrationLayout layout;
        };
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
            CalibrationSession result;
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