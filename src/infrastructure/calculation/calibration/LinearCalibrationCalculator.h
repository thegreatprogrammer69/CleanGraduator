#ifndef CLEANGRADUATOR_LINEARRESULTCALCULATOR_H
#define CLEANGRADUATOR_LINEARRESULTCALCULATOR_H
#include "LinearCalibrationCalculatorConfig.h"
#include "CalibrationCalculatorPorts.h"
#include "domain/fmt/Logger.h"
#include "domain/ports/calibration/calculation/ICalibrationCalculator.h"

namespace infra::calc {
    class LinearCalibrationCalculator : public domain::ports::ICalibrationCalculator {
    public:
        LinearCalibrationCalculator(CalibrationCalculatorPorts, LinearCalibrationCalculatorConfig);
        ~LinearCalibrationCalculator() override;
        domain::common::CalibrationCellComputation compute(const domain::common::CalibrationCalculatorInput&) const override;
    private:
        mutable fmt::Logger logger_;
        LinearCalibrationCalculatorConfig config_;
    };
}

#endif //CLEANGRADUATOR_LINEARRESULTCALCULATOR_H