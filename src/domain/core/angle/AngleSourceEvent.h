#ifndef CLEANGRADUATOR_ANGLESOURCEEVENT_H
#define CLEANGRADUATOR_ANGLESOURCEEVENT_H
#include <variant>
#include "AngleSourceError.h"

namespace domain::common {

    struct AngleSourceStarted {};
    struct AngleSourceStopped {};
    struct AngleSourceFailed {
        AngleSourceError error;
    };

    using AngleSourceEvent = std::variant<
        AngleSourceStarted,
        AngleSourceStopped,
        AngleSourceFailed
    >;
}

#endif //CLEANGRADUATOR_ANGLESOURCEEVENT_H