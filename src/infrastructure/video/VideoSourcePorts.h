#ifndef CLEANGRADUATOR_VIDEOSTREAMPORTS_H
#define CLEANGRADUATOR_VIDEOSTREAMPORTS_H

namespace domain::events {
    class IEventBus;
}

namespace domain::ports {
    class IClock;
    class ILogger;
}

namespace infra::camera {

struct VideoSourcePorts {
    domain::ports::ILogger& logger;
    domain::ports::IClock& clock;
    domain::events::IEventBus& event_bus;
};

}

#endif //CLEANGRADUATOR_VIDEOSTREAMPORTS_H