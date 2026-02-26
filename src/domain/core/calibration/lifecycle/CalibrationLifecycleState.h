#ifndef CLEANGRADUATOR_PROCESSLIFECYCLESTATE_H
#define CLEANGRADUATOR_PROCESSLIFECYCLESTATE_H

namespace domain::common {
    enum class CalibrationLifecycleState {
        Uninitialized,
        Stopped,
        Started
    };
}

#endif //CLEANGRADUATOR_PROCESSLIFECYCLESTATE_H