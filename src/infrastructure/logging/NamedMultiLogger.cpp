#include "NamedMultiLogger.h"

#include <algorithm>

#include "../../application/ports/outbound/logging/ILogSink.h"
#include "../../domain/ports/clock/IClock.h"

static constexpr size_t MAX_LOGS_HISTORY = 482;

using namespace domain::ports;
using namespace application::ports;
using namespace application::models;
using namespace infra::logging;

NamedMultiLogger::NamedMultiLogger(
    IClock &clock,
    std::string name)
        : name_(std::move(name)), clock_(clock)

{
}

NamedMultiLogger::~NamedMultiLogger() {
}

void NamedMultiLogger::addLogger(ILogger &logger) {
    loggers_.push_back(&logger);
}

void NamedMultiLogger::info(const std::string &msg) {
    for (auto &logger : loggers_) {
        logger->info(msg);
    }

    pushLog(LogLevel::Info, msg);
}

void NamedMultiLogger::warn(const std::string &msg) {
    for (auto &logger : loggers_) {
        logger->warn(msg);
    }

    pushLog(LogLevel::Warn, msg);
}

void NamedMultiLogger::error(const std::string &msg) {
    for (auto &logger : loggers_) {
        logger->error(msg);
    }

    pushLog(LogLevel::Error, msg);
}

std::vector<LogEntry> NamedMultiLogger::history() const {
    std::lock_guard lock(logs_mutex_);
    return {logs_.begin(), logs_.end()};
}

void NamedMultiLogger::addSink(ILogSink& sink)
{
    std::lock_guard lock(sinks_mutex_);

    auto it = std::find(sinks_.begin(), sinks_.end(), &sink);
    if (it == sinks_.end()) {
        sinks_.push_back(&sink);
    }
}

void NamedMultiLogger::removeSink(ILogSink& sink)
{
    std::lock_guard lock(sinks_mutex_);

    auto it = std::remove(sinks_.begin(), sinks_.end(), &sink);
    if (it != sinks_.end()) {
        sinks_.erase(it, sinks_.end());
    }
}

void NamedMultiLogger::pushLog(LogLevel level, const std::string& msg)
{
    LogEntry log_entry;
    log_entry.timestamp = clock_.now();
    log_entry.level = level;
    log_entry.message = msg;

    std::vector<ILogSink*> sinks_copy;

    {
        std::lock_guard lock(logs_mutex_);
        if (logs_.size() >= MAX_LOGS_HISTORY) {
            logs_.pop_front();
        }
        logs_.emplace_back(log_entry);
    }

    {
        std::lock_guard lock(sinks_mutex_);
        sinks_copy = sinks_;
    }

    for (auto* sink : sinks_copy) {
        sink->onLog(log_entry);
    }
}

