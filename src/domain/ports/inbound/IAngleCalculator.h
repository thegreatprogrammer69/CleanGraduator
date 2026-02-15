#ifndef UNTITLED_IANGLEMETER_H
#define UNTITLED_IANGLEMETER_H

namespace domain::common {
    class Angle;
    struct AnglemeterInput;
}

namespace domain::ports {
    struct IAngleCalculator {
        IAngleCalculator() = default;
        virtual ~IAngleCalculator() = default;
        virtual common::Angle calculate(const common::AnglemeterInput&) = 0;
    };
}

#endif //UNTITLED_IANGLEMETER_H
