#ifndef CLEANGRADUATOR_PROCESSLIFECYCLESTATE_H
#define CLEANGRADUATOR_PROCESSLIFECYCLESTATE_H

namespace domain::common {
    enum class ProcessLifecycleState {
        Idle,
        Forward,
        Backward,
        Stopping,
        EmergencyStopped
    };
}

#endif //CLEANGRADUATOR_PROCESSLIFECYCLESTATE_H