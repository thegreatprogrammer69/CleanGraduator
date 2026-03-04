#ifndef CLEANGRADUATOR_ANGLESAMPLE_H
#define CLEANGRADUATOR_ANGLESAMPLE_H
#include "domain/core/angle/AngleSourceId.h"

namespace domain::common {
    struct AngleSample {
        AngleSourceId id;
        float time;
        float angle;
    };

}

#endif //CLEANGRADUATOR_ANGLESAMPLE_H