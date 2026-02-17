#ifndef CLEANGRADUATOR_ILOGSINK_H
#define CLEANGRADUATOR_ILOGSINK_H

namespace application::models {
    struct LogEntry;
}

namespace application::ports {
    struct ILogSink {
        virtual ~ILogSink() = default;
        virtual void onLog(const models::LogEntry& log) = 0;
    };
}


#endif //CLEANGRADUATOR_ILOGSINK_H