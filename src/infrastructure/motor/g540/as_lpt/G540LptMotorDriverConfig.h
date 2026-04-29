#ifndef CLEANGRADUATOR_G540LPTCONFIG_H
#define CLEANGRADUATOR_G540LPTCONFIG_H

namespace infra::motors {
    struct G540LptMotorDriverConfig {
        unsigned short lpt_port{};

        unsigned char byte_open_input_flap{};
        unsigned char byte_open_output_flap{};
        unsigned char byte_close_both_flaps{};

        int bit_begin_limit_switch{};
        int bit_end_limit_switch{};

        static constexpr int kDefaultMaxFreqHz = 2000;
        int max_freq_hz{kDefaultMaxFreqHz};
        int min_freq_hz{};
    };
}

#endif //CLEANGRADUATOR_G540LPTCONFIG_H