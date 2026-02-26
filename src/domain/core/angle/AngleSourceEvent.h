#ifndef CLEANGRADUATOR_ANGLESOURCEEVENT_H
#define CLEANGRADUATOR_ANGLESOURCEEVENT_H
#include <variant>
#include "AngleSourceError.h"

namespace domain::common {

    struct AngleSourceEvent {

        struct Started {};
        struct Stopped {};
        struct Failed { AngleSourceError error; };

        using Data = std::variant<
            Started,
            Stopped,
            Failed
        >;

        Data data;
    };
}

#endif //CLEANGRADUATOR_ANGLESOURCEEVENT_H