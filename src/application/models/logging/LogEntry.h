#ifndef CLEANGRADUATOR_LOGENTRY_H
#define CLEANGRADUATOR_LOGENTRY_H
#include <string>

#include "application/LogLevel.h"
#include "domain/core/measurement/Timestamp.h"

namespace application::models {
    struct LogEntry {
        domain::common::Timestamp timestamp;
        LogLevel level;
        std::string message;
    };
}

#endif //CLEANGRADUATOR_LOGENTRY_H