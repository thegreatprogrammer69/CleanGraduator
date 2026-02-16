#ifndef CLEANGRADUATOR_MOTORFAULT_H
#define CLEANGRADUATOR_MOTORFAULT_H

namespace domain::common {
    enum class MotorFault {
        None,
        DriverError,
        EmergencyStop
    };
}

#endif //CLEANGRADUATOR_MOTORFAULT_H