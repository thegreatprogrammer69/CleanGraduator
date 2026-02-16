#ifndef CLEANGRADUATOR_DM5002CONFIG_H
#define CLEANGRADUATOR_DM5002CONFIG_H
#include <string>

namespace infra::pressure {
    struct DM5002RFPressureSensorConfig {
        std::string com_port{};
        int poll_rate = 10;
    };
}

#endif //CLEANGRADUATOR_DM5002CONFIG_H