#ifndef CLEANGRADUATOR_DM5002PRESSURESENSORCONFIG_H
#define CLEANGRADUATOR_DM5002PRESSURESENSORCONFIG_H
#include <string>

namespace infra::pressure {
    struct DM5002PressureSensorConfig {
        std::string com_port{};
        int poll_rate = 10;
    };
}


#endif //CLEANGRADUATOR_DM5002PRESSURESENSORCONFIG_H