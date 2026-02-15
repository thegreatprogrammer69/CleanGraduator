#ifndef CLEANGRADUATOR_LOGSOURCE_H
#define CLEANGRADUATOR_LOGSOURCE_H
#include <string>

namespace application::ports {
    struct ILogSource;
}

namespace application::models {
    struct LogSource {
        std::string name;
        ports::ILogSource& source;
    };
}

#endif //CLEANGRADUATOR_LOGSOURCE_H