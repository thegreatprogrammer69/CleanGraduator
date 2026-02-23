#ifndef CLEANGRADUATOR_FREQUENCYCALCULATOR_H
#define CLEANGRADUATOR_FREQUENCYCALCULATOR_H

namespace infra::calib::stand4 {
    class Stand4FrequencyCalculator {
    public:
        Stand4FrequencyCalculator(
            double divider_min, double divider_max,
            double freq_min, double freq_max,
            double k_upper_dead_zone,
            double k_lower_dead_zone,
            double k_divider_step_percent);

        ~Stand4FrequencyCalculator();

        int frequency(double p_cur, double p_target, double dp_cur, double dp_target);
        void reset();

    private:
        double divider_;

        const double divider_min_;
        const double divider_max_;
        const double freq_min_;
        const double freq_max_;
        const double k_upper_dead_zone_;
        const double k_lower_dead_zone_;
        const double k_divider_step_percent_;
    };
}

#endif //CLEANGRADUATOR_FREQUENCYCALCULATOR_H