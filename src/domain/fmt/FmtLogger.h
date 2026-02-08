#ifndef CLEANGRADUATOR_FMTLOGGER_H
#define CLEANGRADUATOR_FMTLOGGER_H

#include <utility>
#include "domain/ports/outbound/ILogger.h"
#include "fmt.h"


namespace fmt {

    class FmtLogger final {
    public:
        explicit FmtLogger(domain::ports::ILogger& logger)
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
        domain::ports::ILogger& logger_;
    };

}

#endif //CLEANGRADUATOR_FMTLOGGER_H