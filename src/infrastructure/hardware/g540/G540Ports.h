#ifndef CLEANGRADUATOR_P540PORTS_H
#define CLEANGRADUATOR_P540PORTS_H

namespace domain::ports {
    class ILogger;
}

namespace infra::hardware {
    struct G540Ports {
        domain::ports::ILogger& logger;
    };
}

#endif //CLEANGRADUATOR_P540PORTS_H