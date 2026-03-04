#ifndef CLEANGRADUATOR_MOTORLIMITSSTATE_H
#define CLEANGRADUATOR_MOTORLIMITSSTATE_H

namespace domain::common {
    struct MotorLimitsState {
        bool home{false};
        bool end{false};
        bool operator==(const MotorLimitsState& other) const {
            return home == other.home && end == other.end;
        }
    };
}


#endif //CLEANGRADUATOR_MOTORLIMITSSTATE_H