#ifndef CLEANGRADUATOR_MOTORDRIVERSTATE_H
#define CLEANGRADUATOR_MOTORDRIVERSTATE_H

namespace domain::common {
    enum class MotorDriverState {
        Uninitialized,
        Running,
        Stopped,
    };
}

#endif //CLEANGRADUATOR_MOTORDRIVERSTATE_H