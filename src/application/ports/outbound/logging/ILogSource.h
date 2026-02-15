#ifndef CLEANGRADUATOR_ILOGSOURCE_H
#define CLEANGRADUATOR_ILOGSOURCE_H
#include <vector>
#include <string>

#include "../../../models/logging/LogEntry.h"

namespace application::ports {
    struct ILogSink;

    struct ILogSource {
        virtual ~ILogSource() = default;

        virtual const std::vector<models::LogEntry>& history() const = 0;
        virtual void addSink(ILogSink& sink) = 0;
        virtual void removeSink(ILogSink& sink) = 0;
    };

}

#endif //CLEANGRADUATOR_ILOGSOURCE_H