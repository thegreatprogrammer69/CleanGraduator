#ifndef CLEANGRADUATOR_MOTOREVENT_H
#define CLEANGRADUATOR_MOTOREVENT_H

#include <variant>
#include "MotorLimitsState.h"
#include "MotorDirection.h"
#include "MotorDriverError.h"
#include "MotorFlapsState.h"

namespace domain::common {

    struct MotorDriverEvent {


        struct Started {};
        struct Stopped {};
        struct StoppedAtHome {};
        struct StoppedAtEnd {};

        struct Fault {
            MotorDriverError error;
        };

        struct LimitsChanged {
            MotorLimitsState limits;
        };

        struct DirectionChanged {
            MotorDirection direction;
        };

        struct FlapsStateChanged {
            MotorFlapsState state;
        };

        using Data = std::variant<
            Started,
            Stopped,
            StoppedAtHome,
            StoppedAtEnd,
            Fault,
            LimitsChanged,
            DirectionChanged,
            FlapsStateChanged
        >;

        Data data;

        MotorDriverEvent(Data data) : data(std::move(data)) {}
    };

}

#endif //CLEANGRADUATOR_MOTOREVENT_H