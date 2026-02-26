#ifndef CLEANGRADUATOR_PRESSURESOURCEEVENT_H
#define CLEANGRADUATOR_PRESSURESOURCEEVENT_H
#include <variant>

#include "PressurePacket.h"
#include "PressureSourceError.h"

namespace domain::common {

    struct PressureSourceEvent {

        struct Opened {};

        struct Failed {
            PressureSourceError error;
        };

        struct Closed {};

        using Data = std::variant<
            Opened,
            Failed,
            Closed
        >;

        Data data;
    };

}

#endif //CLEANGRADUATOR_PRESSURESOURCEEVENT_H