#ifndef CLEANGRADUATOR_ANGLECALCULATORFACTORY_H
#define CLEANGRADUATOR_ANGLECALCULATORFACTORY_H

#include <memory>
#include <string>
#include <vector>

#include "../../domain/ports/angle/IAngleCalculator.h"
#include "infrastructure/calculation/angle/AnglemeterPorts.h"

namespace infra::repo {
    class AngleCalculatorFactory final {
    public:
        explicit AngleCalculatorFactory(const std::string& ini_path, calc::AnglemeterPorts ports);
        ~AngleCalculatorFactory();

        std::unique_ptr<domain::ports::IAngleCalculator> load();

    private:
        std::string ini_path_;
        calc::AnglemeterPorts ports_;
    };
}

#endif //CLEANGRADUATOR_ANGLECALCULATORFACTORY_H