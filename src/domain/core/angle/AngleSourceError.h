#ifndef CLEANGRADUATOR_ANGLESOURCEERROR_H
#define CLEANGRADUATOR_ANGLESOURCEERROR_H
#include <string>
#include "AngleSourceId.h"

namespace domain::common {
    struct AngleSourceError {
        AngleSourceId id;
        std::string reason;
        bool is_error;
    };
}

#endif //CLEANGRADUATOR_ANGLESOURCEERROR_H