#ifndef CLEANGRADUATOR_LINEARRESULTCALCULATOR_H
#define CLEANGRADUATOR_LINEARRESULTCALCULATOR_H
#include "LinearCalibrationCalculatorConfig.h"
#include "CalibrationCalculatorPorts.h"
#include "domain/fmt/Logger.h"
#include "../../../domain/ports/calibration/ICalibrationCalculator.h"

namespace infra::calc {
    class LinearCalibrationCalculator : public domain::ports::ICalibrationCalculator {
    public:
        ~LinearCalibrationCalculator() override;
        LinearCalibrationCalculator(CalibrationCalculatorPorts, LinearCalibrationCalculatorConfig);
        domain::common::CalibrationResult calculate(const domain::common::CalibrationInput&) const override;
    private:
        LinearCalibrationCalculatorConfig config_;
        mutable fmt::Logger logger_;
    };
}

#endif //CLEANGRADUATOR_LINEARRESULTCALCULATOR_H