#ifndef CLEANGRADUATOR_RESULTANGLE_H
#define CLEANGRADUATOR_RESULTANGLE_H
#include <string>

#include "ResultProblemCategory.h"

namespace domain::common {
    struct ResultAngle {
        double angle;
        ResultProblemCategory problem;
        std::string error;
    };
}

#endif //CLEANGRADUATOR_RESULTANGLE_H