#ifndef UNTITLED_ILOGGER_H
#define UNTITLED_ILOGGER_H

#include <string>


namespace domain::ports {
    struct ILogger {
        virtual ~ILogger() = default;
        virtual void info(const std::string& msg)  = 0;
        virtual void warn(const std::string& msg) = 0;
        virtual void error(const std::string& msg) = 0;
    };
}

#endif //UNTITLED_ILOGGER_H