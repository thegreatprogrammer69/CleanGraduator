#ifndef CLEANGRADUATOR_PRESSURESOURCEEVENT_H
#define CLEANGRADUATOR_PRESSURESOURCEEVENT_H
#include <variant>

#include "PressurePacket.h"
#include "PressureSourceError.h"

namespace domain::common {

    struct PressureSourceEvent {

        struct Opened {};

        struct OpenFailed {
            PressureSourceError error;
        };

        struct Failed {
            PressureSourceError error;
        };

        struct Closed {};

        using Data = std::variant<
            Opened,
            OpenFailed,
            Failed,
            Closed
        >;

        Data data;

        PressureSourceEvent(Data data) : data(std::move(data)) {}
    };

}

#endif //CLEANGRADUATOR_PRESSURESOURCEEVENT_H