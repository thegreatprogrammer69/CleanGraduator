#ifndef CLEANGRADUATOR_IANGLEMETERFACTORY_H
#define CLEANGRADUATOR_IANGLEMETERFACTORY_H

#include <memory>
#include <vector>

namespace domain::ports {
    struct IAngleCalculator;
}

namespace application::ports {
    struct IAnglemeterFactory {
        virtual std::unique_ptr<domain::ports::IAngleCalculator> create() = 0;
        virtual ~IAnglemeterFactory() = default;
    };
}

#endif //CLEANGRADUATOR_IANGLEMETERFACTORY_H