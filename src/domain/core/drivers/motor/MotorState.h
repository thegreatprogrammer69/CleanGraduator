#ifndef CLEANGRADUATOR_MOTORSTATE_H
#define CLEANGRADUATOR_MOTORSTATE_H

namespace domain::common {
    enum class MotorState {
        Idle,
        RunningForward,
        RunningBackward,
        EmergencyStopped
    };
}

#endif //CLEANGRADUATOR_MOTORSTATE_H