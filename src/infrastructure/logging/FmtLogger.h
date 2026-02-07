#ifndef CLEANGRADUATOR_FMTLOGGER_H
#define CLEANGRADUATOR_FMTLOGGER_H

#include <string>

#include "domain/ports/outbound/ILogger.h"
#include "infrastructure/fmt/fmt.h"

namespace domain::ports {

    class FmtLogger {
    public:
        explicit FmtLogger(ILogger& logger)
            : logger_(logger) {}

        template <typename... Args>
        void info(const std::string& format, const Args&... args) {
            logger_.info(fmt::fmt(format, args...));
        }

        template <typename... Args>
        void warn(const std::string& format, const Args&... args) {
            logger_.warn(fmt::fmt(format, args...));
        }

        template <typename... Args>
        void error(const std::string& format, const Args&... args) {
            logger_.error(fmt::fmt(format, args...));
        }

    private:
        ILogger& logger_;
    };

} // namespace domain::ports

}


#endif //CLEANGRADUATOR_FMTLOGGER_H