#ifndef CLEANGRADUATOR_FMTLOGGER_H
#define CLEANGRADUATOR_FMTLOGGER_H

#include <utility>
#include "domain/ports/outbound/ILogger.h"
#include "fmt.h"


namespace fmt {

    class Logger final {
    public:
        explicit Logger(domain::ports::ILogger& logger)
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
            last_error_ = fmt::fmt(format, args...);
            logger_.error(last_error_);
        }

        const std::string& lastError() {
            auto last_error = last_error_;
            last_error_.clear();
            return last_error_;
        }

    private:
        domain::ports::ILogger& logger_;
        std::string last_error_;
    };

}

#endif //CLEANGRADUATOR_FMTLOGGER_H