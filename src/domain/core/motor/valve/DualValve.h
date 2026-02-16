#ifndef CLEANGRADUATOR_DUALVALVE_H
#define CLEANGRADUATOR_DUALVALVE_H

#include "ValvePosition.h"

namespace domain::ports {
    struct IDualValveDriver;
}

namespace domain::common {

    class DualValve {
    public:
        explicit DualValve(ports::IDualValveDriver& driver);

        ValvePosition position() const noexcept;

        bool isClosed() const noexcept;
        bool isIntakeOpen() const noexcept;
        bool isExhaustOpen() const noexcept;

        void openIntake();
        void openExhaust();
        void close();

    private:
        ports::IDualValveDriver& driver_;
        ValvePosition state_{ValvePosition::Closed};
    };

}

#endif //CLEANGRADUATOR_DUALVALVE_H