#ifndef CLEANGRADUATOR_LINEARRESULTCALCULATORCONFIG_H
#define CLEANGRADUATOR_LINEARRESULTCALCULATORCONFIG_H

namespace infra::calc {
    struct LinearCalibrationCalculatorConfig {
        int min_pressure_values_in_series = 3;        // меньше — ошибка
        int few_pressure_values_in_series = 5;        // меньше — warning

        int min_angle_values_in_series = 3;           // меньше — ошибка
        int few_angle_values_in_series = 10;          // меньше — warning

    };
}

#endif //CLEANGRADUATOR_LINEARRESULTCALCULATORCONFIG_H