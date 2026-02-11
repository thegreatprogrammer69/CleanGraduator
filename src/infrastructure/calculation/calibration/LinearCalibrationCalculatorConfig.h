#ifndef CLEANGRADUATOR_LINEARRESULTCALCULATORCONFIG_H
#define CLEANGRADUATOR_LINEARRESULTCALCULATORCONFIG_H

namespace infra::calc {
    struct LinearCalibrationCalculatorConfig {
        int min_pressure_values_in_series = 3;
        int min_angle_values_in_series = 7;
        double overlap = 0.02;
    };
}

#endif //CLEANGRADUATOR_LINEARRESULTCALCULATORCONFIG_H