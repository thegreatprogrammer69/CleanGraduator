#ifndef CLEANGRADUATOR_ANGLESOURCEPORTS_H
#define CLEANGRADUATOR_ANGLESOURCEPORTS_H

namespace domain::ports {
    struct IAngleCalculator;
}

namespace domain::ports {
    struct ILogger;
    struct IVideoSource;
}

namespace infra::angle {
    struct AngleSourcePorts {
        domain::ports::ILogger& logger;
        domain::ports::IAngleCalculator& anglemeter;
        domain::ports::IVideoSource& video_source;
    };
}

#endif //CLEANGRADUATOR_ANGLESOURCEPORTS_H