#ifndef CLEANGRADUATOR_MOTORFAULT_H
#define CLEANGRADUATOR_MOTORFAULT_H

namespace domain::common {
    enum class MotorFault {
        None,
        WatchdogTimeout,
        ForwardLimit,
        BackwardLimit,
        HardwareFailure
    };
}

#endif //CLEANGRADUATOR_MOTORFAULT_H