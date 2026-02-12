#ifndef CLEANGRADUATOR_IPRESSUREACTUATOROBSERVER_H
#define CLEANGRADUATOR_IPRESSUREACTUATOROBSERVER_H

namespace domain::ports {
    struct IPressureActuatorObserver {
        virtual ~IPressureActuatorObserver() = default;
        virtual bool onStarted() = 0;
        virtual void onStopped() = 0;
        virtual void onEmergencyStop() = 0;
    };
}

#endif //CLEANGRADUATOR_IPRESSUREACTUATOROBSERVER_H