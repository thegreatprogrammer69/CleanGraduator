#ifndef CLEANGRADUATOR_LOGSOURCE_H
#define CLEANGRADUATOR_LOGSOURCE_H
#include <string>

namespace application::ports {
    struct ILogSource;
}

namespace application::models {
    struct LogSource {
        std::string name;
        ports::ILogSource* source;

        bool operator == (const LogSource& o) const {
            return name == o.name;
        }

        bool operator != (const LogSource& o) const {
            return !(*this == o);
        }
    };
}

#endif //CLEANGRADUATOR_LOGSOURCE_H