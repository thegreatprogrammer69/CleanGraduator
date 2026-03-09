#ifndef CLEANGRADUATOR_ANGLESAMPLE_H
#define CLEANGRADUATOR_ANGLESAMPLE_H
#include "domain/core/angle/SourceId.h"

namespace domain::common {
    struct AngleSample {
        SourceId id;
        float time;
        float angle;
    };
}

#endif //CLEANGRADUATOR_ANGLESAMPLE_H