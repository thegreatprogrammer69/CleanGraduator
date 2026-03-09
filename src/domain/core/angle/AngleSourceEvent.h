#ifndef CLEANGRADUATOR_ANGLESOURCEEVENT_H
#define CLEANGRADUATOR_ANGLESOURCEEVENT_H
#include <variant>
#include "AngleSourceError.h"

namespace domain::common {

    struct AngleSourceEvent {

        struct Started { SourceId id; };
        struct Stopped { SourceId id; };
        struct Failed { SourceId id; AngleSourceError error; };

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