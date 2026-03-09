#ifndef CLEANGRADUATOR_MOTORCONTROLINTERACTOR_H
#define CLEANGRADUATOR_MOTORCONTROLINTERACTOR_H
#include <vector>

#include "domain/core/drivers/motor/MotorDirection.h"
#include "domain/ports/drivers/motor/IMotorDriverObserver.h"


namespace domain::ports {
    struct IMotorDriver;
}

namespace application::orchestrators {
    struct IMotorControlObserver;

    class MotorControlInteractor final
    {
    public:
        explicit MotorControlInteractor(domain::ports::IMotorDriver& driver);

        ~MotorControlInteractor();

        void start(int frequency, domain::common::MotorDirection dir);
        void stop();

    private:
        domain::ports::IMotorDriver& driver_;
    };
}


#endif //CLEANGRADUATOR_MOTORCONTROLINTERACTOR_H