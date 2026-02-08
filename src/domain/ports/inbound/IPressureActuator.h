#ifndef CLEANGRADUATOR_IPRESSUREACTUATOR_H
#define CLEANGRADUATOR_IPRESSUREACTUATOR_H

namespace domain::common {
    struct PressureActuatorInput;
}

namespace domain::ports {
    struct IPressureActuatorObserver;

    struct IPressureActuator {
        virtual ~IPressureActuator() = default;

        bool start(const common::PressureActuatorInput &);
        void stop();
        void emergencyStop();

        virtual void subscribe(IPressureActuatorObserver&) = 0;
    };
}

#endif //CLEANGRADUATOR_IPRESSUREACTUATOR_H