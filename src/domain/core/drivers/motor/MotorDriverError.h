#ifndef CLEANGRADUATOR_MOTORERROR_H
#define CLEANGRADUATOR_MOTORERROR_H

#include <string>

namespace domain::common {
    struct MotorDriverError {
        std::string message;
        void reset() { message.clear(); }
    };
}

#endif //CLEANGRADUATOR_MOTORERROR_H