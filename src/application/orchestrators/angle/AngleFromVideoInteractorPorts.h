#ifndef CLEANGRADUATOR_ANGLEFROMVIDEOINTERACTORPORTS_H
#define CLEANGRADUATOR_ANGLEFROMVIDEOINTERACTORPORTS_H

namespace domain::ports {
    struct IVideoSource;
}

namespace domain::ports {
    struct ILogger;
    struct IAngleCalculator;
}

namespace application::orchestrators {
    struct AngleFromVideoInteractorPorts {
        domain::ports::ILogger& logger;
        domain::ports::IAngleCalculator& anglemeter;
        domain::ports::IVideoSource& video_source;
    };
}

#endif //CLEANGRADUATOR_ANGLEFROMVIDEOINTERACTORPORTS_H