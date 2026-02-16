#ifndef CLEANGRADUATOR_P540PORTS_H
#define CLEANGRADUATOR_P540PORTS_H

namespace domain::ports {
    class ILogger;
}

namespace infra::motors {
    struct G540LptMotorDriverPorts {
        domain::ports::ILogger& logger;
    };
}

#endif //CLEANGRADUATOR_P540PORTS_H