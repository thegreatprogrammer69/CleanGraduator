#ifndef CLEANGRADUATOR_CALIBRATIONPOLICYSERVICE_H
#define CLEANGRADUATOR_CALIBRATIONPOLICYSERVICE_H

#include "domain/core/calibration/CalibrationInput.h"
#include "domain/core/calibration/CalibrationResult.h"

namespace domain::ports {
    class ICalibrationCalculator;
    struct ILogger;
}

namespace application::services {

class CalibrationPolicyService {
public:
    CalibrationPolicyService(domain::ports::ICalibrationCalculator& calculator, domain::ports::ILogger& logger);

    domain::common::CalibrationResult calculate(const domain::common::CalibrationInput& input) const;

private:
    domain::ports::ICalibrationCalculator& calculator_;
    domain::ports::ILogger& logger_;
};

}

#endif //CLEANGRADUATOR_CALIBRATIONPOLICYSERVICE_H

