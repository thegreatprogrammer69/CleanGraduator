#ifndef CLEANGRADUATOR_MOTIONABORTREASON_H
#define CLEANGRADUATOR_MOTIONABORTREASON_H
#include <string>

namespace domain::ports {
    enum class MotionAbortReasonType {
        EmergencyStop,
        LimitFault,
        MotorFault,
        UserAbort
    };
    struct MotionAbortReason {
        MotionAbortReason() = delete;
        explicit MotionAbortReason(MotionAbortReasonType type) : type(type) {}
        MotionAbortReason(MotionAbortReasonType type, std::string message) : type(type), message(message) {}
        MotionAbortReasonType type;
        std::string message;
    };
}

#endif //CLEANGRADUATOR_MOTIONABORTREASON_H