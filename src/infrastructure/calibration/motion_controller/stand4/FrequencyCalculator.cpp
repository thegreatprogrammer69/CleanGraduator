#include "FrequencyCalculator.h"

#include <algorithm>
#include <cmath>

infra::calib::stand4::FrequencyCalculator::FrequencyCalculator(double divider_min, double divider_max, double freq_min,
    double freq_max, double k_upper_dead_zone, double k_lower_dead_zone, double k_divider_step_percent)
        : divider_min_(divider_min), divider_max_(divider_max), freq_min_(freq_min), freq_max_(freq_max)
        , k_upper_dead_zone_(k_upper_dead_zone), k_lower_dead_zone_(k_lower_dead_zone)
        , k_divider_step_percent_(k_divider_step_percent) {

}

infra::calib::stand4::FrequencyCalculator::~FrequencyCalculator() {
}

int infra::calib::stand4::FrequencyCalculator::frequency(double p_cur, double p_target, double dp_cur, double dp_target) {
    if (p_target == 0) return freq_min_;

    const double abs_dp_cur = std::fabs(dp_cur);
    const double abs_dp_target = std::fabs(dp_target);

    const double divider_step = (divider_max_ - divider_min_) * k_divider_step_percent_ / 100.;

    if (abs_dp_cur > abs_dp_target * k_upper_dead_zone_) {
        divider_ += divider_step;
    }
    else if (abs_dp_cur < abs_dp_target * k_lower_dead_zone_) {
        divider_ -= divider_step;
    }

    divider_ = std::clamp(divider_, divider_min_, divider_max_);

    const double freq_base = (freq_max_ - freq_min_) * (p_cur / p_target) + freq_min_;

    return std::clamp(freq_base / divider_, freq_min_, freq_max_) ;
}
