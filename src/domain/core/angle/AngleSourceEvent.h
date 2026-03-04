#ifndef CLEANGRADUATOR_ANGLESOURCEEVENT_H
#define CLEANGRADUATOR_ANGLESOURCEEVENT_H
#include <variant>
#include "AngleSourceError.h"

namespace domain::common {

    struct AngleSourceEvent {

        struct Started { AngleSourceId id; };
        struct Stopped { AngleSourceId id; };
        struct Failed { AngleSourceId id; AngleSourceError error; };

        using Data = std::variant<
            Started,
            Stopped,
            Failed
        >;

        Data data;

        AngleSourceEvent(Data data) : data(std::move(data)) {};
    };
}

#endif //CLEANGRADUATOR_ANGLESOURCEEVENT_H