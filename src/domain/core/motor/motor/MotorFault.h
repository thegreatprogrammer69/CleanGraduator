#ifndef CLEANGRADUATOR_MOTORFAULT_H
#define CLEANGRADUATOR_MOTORFAULT_H
#include <string>


namespace domain::common {
    enum class MotorFaultType {
        None,
        DriverError,
        EmergencyStop
    };
    struct MotorFault {
        MotorFaultType type = MotorFaultType::None;
        std::string error;
        bool operator==(const MotorFault& rhs) const {
            return type == rhs.type && error == rhs.error;
        }
    };
}

#endif //CLEANGRADUATOR_MOTORFAULT_H