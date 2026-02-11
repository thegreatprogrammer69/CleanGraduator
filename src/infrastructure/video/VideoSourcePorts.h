#ifndef CLEANGRADUATOR_VIDEOSTREAMPORTS_H
#define CLEANGRADUATOR_VIDEOSTREAMPORTS_H


namespace domain::ports {
    class IClock;
    class ILogger;
}

namespace infra::camera {

struct VideoSourcePorts {
    domain::ports::ILogger& logger;
    domain::ports::IClock& clock;
};

}

#endif //CLEANGRADUATOR_VIDEOSTREAMPORTS_H