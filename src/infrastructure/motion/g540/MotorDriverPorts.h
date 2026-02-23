#ifndef CLEANGRADUATOR_P540PORTS_H
#define CLEANGRADUATOR_P540PORTS_H

namespace domain::ports {
    class ILogger;
}

namespace infra::motors {
    struct MotorDriverPorts {
        domain::ports::ILogger& logger;
    };
}

#endif //CLEANGRADUATOR_P540PORTS_H