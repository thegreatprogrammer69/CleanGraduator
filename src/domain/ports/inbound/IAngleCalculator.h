#ifndef UNTITLED_IANGLEMETER_H
#define UNTITLED_IANGLEMETER_H
#include "domain/core/measurement/Angle.h"
#include "domain/core/inputs/AnglemeterInput.h"

namespace domain::ports {
    struct IAngleCalculator {
        IAngleCalculator() = default;
        virtual ~IAngleCalculator() = default;
        virtual common::Angle calculate(const common::AnglemeterInput&) = 0;
    };
}

#endif //UNTITLED_IANGLEMETER_H