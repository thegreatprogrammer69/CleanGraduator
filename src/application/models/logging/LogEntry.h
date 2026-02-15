#ifndef CLEANGRADUATOR_LOGENTRY_H
#define CLEANGRADUATOR_LOGENTRY_H
#include <string>

#include "application/LogLevel.h"
#include "domain/core/measurement/Timestamp.h"

namespace application::models {
    struct LogEntry {
        domain::common::Timestamp timestamp{};
        LogLevel level = LogLevel::Info;
        std::string message{};

        bool operator==(const LogEntry& other) const {
            return timestamp == other.timestamp;
        }
    };
}

#endif //CLEANGRADUATOR_LOGENTRY_H