#ifndef UNTITLED_FILELOGGER_H
#define UNTITLED_FILELOGGER_H

#include "domain/ports/logging/ILogger.h"
#include <atomic>
#include <fstream>
#include <mutex>

namespace infra::logging {

class FileLogger final : public domain::ports::ILogger {
public:
    explicit FileLogger(const std::string& filePath);

    void setEnabled(bool enabled);

    void info(const std::string& msg) override;
    void warn(const std::string& msg) override;
    void error(const std::string& msg) override;

private:
    void log(const std::string& level, const std::string& msg);

    std::ofstream file_;
    std::mutex mutex_;
    std::atomic<bool> enabled_{true};
};

}

#endif //UNTITLED_FILELOGGER_H
