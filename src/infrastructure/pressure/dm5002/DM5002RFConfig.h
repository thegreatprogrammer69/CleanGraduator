#ifndef CLEANGRADUATOR_DM5002CONFIG_H
#define CLEANGRADUATOR_DM5002CONFIG_H
#include <string>

struct DM5002RFConfig {
    std::string com_port;
    int poll_rate;
};

#endif //CLEANGRADUATOR_DM5002CONFIG_H