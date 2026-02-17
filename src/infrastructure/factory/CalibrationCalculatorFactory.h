#ifndef CLEANGRADUATOR_CALIBRATIONCALCULATORFACTORY_H
#define CLEANGRADUATOR_CALIBRATIONCALCULATORFACTORY_H

#include <memory>
#include <string>
#include <vector>

#include "../../domain/ports/calibration/calculation/ICalibrationCalculator.h"
#include "infrastructure/calculation/calibration/CalibrationCalculatorPorts.h"

namespace infra::repo {

    class CalibrationCalculatorFactory final {
    public:
        explicit CalibrationCalculatorFactory(
            const std::string& ini_path,
            calc::CalibrationCalculatorPorts ports);

        ~CalibrationCalculatorFactory();

        std::unique_ptr<domain::ports::ICalibrationCalculator> load();

    private:
        std::string ini_path_;
        calc::CalibrationCalculatorPorts ports_;
    };

}

#endif // CLEANGRADUATOR_CALIBRATIONCALCULATORFACTORY_H
