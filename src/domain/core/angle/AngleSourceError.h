#ifndef CLEANGRADUATOR_ANGLESOURCEERROR_H
#define CLEANGRADUATOR_ANGLESOURCEERROR_H
#include <string>
#include "SourceId.h"

namespace domain::common {
    struct AngleSourceError {
        SourceId id;
        std::string error;
    };
}

#endif //CLEANGRADUATOR_ANGLESOURCEERROR_H