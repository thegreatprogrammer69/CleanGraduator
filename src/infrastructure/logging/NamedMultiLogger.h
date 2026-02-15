#ifndef CLEANGRADUATOR_NAMEDMULTILOGGER_H
#define CLEANGRADUATOR_NAMEDMULTILOGGER_H
#include <deque>
#include <mutex>
#include <vector>

#include "../../application/models/logging/LogEntry.h"
#include "application/ports/outbound/logging/ILogSource.h"
#include "domain/ports/outbound/ILogger.h"

namespace domain::ports {
    struct IClock;
}

namespace application::ports {
    struct ILogSink;
}

namespace infra::logging {
    class NamedMultiLogger final : public domain::ports::ILogger, public application::ports::ILogSource {
    public:
        explicit NamedMultiLogger(domain::ports::IClock& clock, std::string name);
        ~NamedMultiLogger() override;

        void addLogger(ILogger& logger);

        void info(const std::string &msg) override;
        void warn(const std::string &msg) override;
        void error(const std::string &msg) override;

        const std::vector<application::models::LogEntry>& history() const override;
        void addSink(application::ports::ILogSink& sink) override;
        void removeSink(application::ports::ILogSink& sink) override;

    private:
        void pushLog(application::models::LogLevel level, const std::string &msg);

    private:
        const std::string name_;
        std::vector<ILogger*> loggers_;
        domain::ports::IClock& clock_;

        mutable std::mutex logs_mutex_;
        std::deque<application::models::LogEntry> logs_;

        std::mutex sinks_mutex_;
        std::vector<application::ports::ILogSink*> sinks_;
    };
}

#endif //CLEANGRADUATOR_NAMEDMULTILOGGER_H