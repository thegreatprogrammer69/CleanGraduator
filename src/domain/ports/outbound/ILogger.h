#ifndef UNTITLED_ILOGGER_H
#define UNTITLED_ILOGGER_H

#include <string>
#include "domain/fmt/fmt.h"


namespace domain::ports {
    struct ILogger {
        virtual ~ILogger() = default;
        virtual void info(const std::string& msg) = 0;
        virtual void warn(const std::string& msg) = 0;
        virtual void error(const std::string& msg) = 0;

        template <typename... Args>
        void info(fmt::format_string<Args...> format, Args&&... args) {
            info(fmt::format(format, std::forward<Args>(args)...));
        }

        template <typename... Args>
        void warn(fmt::format_string<Args...> format, Args&&... args) {
            warn(fmt::format(format, std::forward<Args>(args)...));
        }

        template <typename... Args>
        void error(fmt::format_string<Args...> format, Args&&... args) {
            error(fmt::format(format, std::forward<Args>(args)...));
        }

    };
}

#endif //UNTITLED_ILOGGER_H