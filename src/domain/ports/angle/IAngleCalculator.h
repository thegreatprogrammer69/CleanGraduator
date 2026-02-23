#ifndef UNTITLED_IANGLEMETER_H
#define UNTITLED_IANGLEMETER_H
#include "domain/core/measurement/Angle.h"
#include "domain/core/angle/AngleCalculatorInput.h"

namespace domain::ports {
    struct IAngleCalculator {
        IAngleCalculator() = default;
        virtual ~IAngleCalculator() = default;
        virtual common::Angle calculate(const common::AngleCalculatorInput&) = 0;
    };
}

#endif //UNTITLED_IANGLEMETER_H