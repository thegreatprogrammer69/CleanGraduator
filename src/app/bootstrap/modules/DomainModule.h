#ifndef CLEANGRADUATOR_DOMAINMODULE_H
#define CLEANGRADUATOR_DOMAINMODULE_H

#include <memory>
#include <string>

#include "application/orchestrators/AngleFromVideoInteractorPorts.h"

namespace application::interactors {
class AngleFromVideoInteractor;
}




struct DomainModule {
    struct AnglemeterConfig {
        int brightLim{150};
        int maxPairs{6};
        int scanStep{2};
    };

    static AnglemeterConfig loadAnglemeterConfig(const std::string& configDirectory);

    static std::unique_ptr<domain::ports::IAngleCalculator> createAnglemeter(
        domain::ports::ILogger& logger,
        const AnglemeterConfig& config);

    static std::unique_ptr<application::interactors::AngleFromVideoInteractor> createAngleFromVideoInteractor(
        domain::ports::ILogger& logger,
        domain::ports::IAngleCalculator& calculator);
};

#endif //CLEANGRADUATOR_DOMAINMODULE_H
