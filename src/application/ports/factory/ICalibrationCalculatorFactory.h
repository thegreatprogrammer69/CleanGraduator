#ifndef CLEANGRADUATOR_IRESULTCALCULATORFACTORY_H
#define CLEANGRADUATOR_IRESULTCALCULATORFACTORY_H

#include <memory>
#include <vector>

namespace domain::ports {
    struct IAngleCalculator;
}

namespace application::ports {
    struct IAnglemeterFactory {
        virtual std::unique_ptr<domain::ports::IResultCalculator> create() = 0;
        virtual ~IAnglemeterFactory() = default;
    };
}

#endif //CLEANGRADUATOR_IRESULTCALCULATORFACTORY_H