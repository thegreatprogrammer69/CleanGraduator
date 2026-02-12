#ifndef CLEANGRADUATOR_VIDEOOVERLAYPORTS_H
#define CLEANGRADUATOR_VIDEOOVERLAYPORTS_H

namespace domain::ports {
    struct IAngleCalculator;
}

namespace infra::overlay {
    struct VideoOverlayPorts {
        domain::ports::IAngleCalculator& anglemeter;
    };
}

#endif //CLEANGRADUATOR_VIDEOOVERLAYPORTS_H