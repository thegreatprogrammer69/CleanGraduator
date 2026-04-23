#ifndef CLEANGRADUATOR_CALIBRATIONVERDICT_H
#define CLEANGRADUATOR_CALIBRATIONVERDICT_H
#include <string>
#include <vector>
#include <variant>
#include "../recording/CalibrationSessionId.h"
#include "domain/core/calibration/common/CalibrationAudioCue.h"
#include "domain/core/drivers/motor/MotorFlapsState.h"
#include "domain/core/drivers/motor/MotorDirection.h"

namespace domain::common {

    struct CalibrationStrategyVerdict {

        struct BeginSession { CalibrationSessionId id; };
        struct EndSession {};

        struct MotorSetFrequency { int freq; };
        struct MotorSetDirection { MotorDirection direction; };
        struct MotorSetFlaps { MotorFlapsState state; };
        struct MotorStart {};
        struct MotorStop {};
        struct StatusText { std::string text; };
        struct PlaySound { CalibrationAudioCue cue; };

        struct Complete {};
        struct Fault { std::string error; };

        using Command = std::variant<
            BeginSession, EndSession,
            MotorSetFrequency, MotorSetDirection,
            MotorSetFlaps, MotorStart, MotorStop, StatusText, PlaySound,
            Complete, Fault
        >;

        std::vector<Command> commands;
    };

}

#endif //CLEANGRADUATOR_CALIBRATIONVERDICT_H
