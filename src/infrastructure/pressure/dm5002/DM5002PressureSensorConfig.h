#ifndef CLEANGRADUATOR_DM5002PRESSURESENSORCONFIG_H
#define CLEANGRADUATOR_DM5002PRESSURESENSORCONFIG_H

#include <string>

namespace infra::pressure {

    struct DM5002PressureSensorConfig {
        enum class Protocol {
            Standard, // 12-byte request, 19-byte response (unit @13, float @14..17)
            RF        // 2-byte request, 5-byte response (unit @0,  float @1..4)
        };

        std::string com_port{};
        int poll_rate = 10;                 // Hz (samples per second). If <= 0 -> fallback to 10 Hz
        Protocol protocol = Protocol::Standard;
        int max_consecutive_failures = 3;   // stop after N invalid reads
    };

} // namespace infra::pressure

#endif // CLEANGRADUATOR_DM5002PRESSURESENSORCONFIG_H