#ifndef UNTITLED_CONSOLELOGGER_H
#define UNTITLED_CONSOLELOGGER_H
#include <domain/ports/outbound/ILogger.h>
#include <iostream>

namespace infra::logging {

class ConsoleLogger final : public ILogger {
public:
    void info(const std::string& msg) override;
    void warn(const std::string& msg) override;
    void error(const std::string& msg) override;
};

}

#endif //UNTITLED_CONSOLELOGGER_H