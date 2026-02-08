#ifndef CLEANGRADUATOR_LINEARRESULTCALCULATOR_H
#define CLEANGRADUATOR_LINEARRESULTCALCULATOR_H
#include "LinearResultCalculatorConfig.h"
#include "LinearResultCalculatorPorts.h"
#include "domain/fmt/FmtLogger.h"
#include "domain/ports/inbound/ICalibrationCalculator.h"

namespace infra::calculation {
    class LinearResultCalculator : public domain::ports::ICalibrationCalculator {
        LinearResultCalculator(LinearResultCalculatorPorts, LinearResultCalculatorConfig);
        domain::common::CalibrationResult calculate(const domain::common::CalibrationInput&) const override;
    private:
        LinearResultCalculatorConfig config_;
        mutable fmt::FmtLogger logger_;
    };
}

#endif //CLEANGRADUATOR_LINEARRESULTCALCULATOR_H