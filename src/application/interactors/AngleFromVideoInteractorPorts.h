#ifndef CLEANGRADUATOR_ANGLEFROMVIDEOINTERACTORPORTS_H
#define CLEANGRADUATOR_ANGLEFROMVIDEOINTERACTORPORTS_H

namespace domain::ports {
    struct ILogger;
    struct IAngleCalculator;
}

namespace application::interactors {
    struct AngleFromVideoInteractorPorts {
        domain::ports::ILogger& logger;
        domain::ports::IAngleCalculator& anglemeter;
    };
}

#endif //CLEANGRADUATOR_ANGLEFROMVIDEOINTERACTORPORTS_H