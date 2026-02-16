#ifndef CLEANGRADUATOR_PRESSURESENSORPORTS_H
#define CLEANGRADUATOR_PRESSURESENSORPORTS_H


namespace domain::ports {
    class ILogger;
    class IClock;
}

namespace infra::pressure {
    struct PressureSourcePorts {
        domain::ports::ILogger& logger;
        domain::ports::IClock& clock;
    };
}

#endif //CLEANGRADUATOR_PRESSURESENSORPORTS_H