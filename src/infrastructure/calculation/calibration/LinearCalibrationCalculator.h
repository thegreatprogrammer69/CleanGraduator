#ifndef CLEANGRADUATOR_LINEARRESULTCALCULATOR_H
#define CLEANGRADUATOR_LINEARRESULTCALCULATOR_H
#include "LinearCalibrationCalculatorConfig.h"
#include "CalibrationCalculatorPorts.h"
#include "domain/fmt/Logger.h"
#include "domain/ports/calibration/calculation/ICalibrationCalculator.h"
#include "domain/ports/calibration/result/IResultStore.h"

namespace infra::calc {
    class LinearCalibrationCalculator : public domain::ports::ICalibrationCalculator {
    public:
        ~LinearCalibrationCalculator() override;
        LinearCalibrationCalculator(CalibrationCalculatorPorts, LinearCalibrationCalculatorConfig);
        domain::common::CalibrationCellComputation compute(const domain::common::CalibrationCalculatorInput&) const override;
    private:
        LinearCalibrationCalculatorConfig config_;
        mutable fmt::Logger logger_;
    };
}

#endif //CLEANGRADUATOR_LINEARRESULTCALCULATOR_H