#ifndef CLEANGRADUATOR_PRESSURESENSORPORTS_H
#define CLEANGRADUATOR_PRESSURESENSORPORTS_H


namespace domain::ports {
    class ILogger;
    class IClock;
}

struct PressureSensorPorts {
    domain::ports::ILogger& logger;
    domain::ports::IClock& session_clock;
};

#endif //CLEANGRADUATOR_PRESSURESENSORPORTS_H