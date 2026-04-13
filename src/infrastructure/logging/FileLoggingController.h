#ifndef CLEANGRADUATOR_FILELOGGINGCONTROLLER_H
#define CLEANGRADUATOR_FILELOGGINGCONTROLLER_H

#include <vector>

#include "application/ports/logging/IFileLoggingControl.h"

namespace infra::logging {
    class FileLogger;

    class FileLoggingController final : public application::ports::IFileLoggingControl {
    public:
        explicit FileLoggingController(bool enabled);

        void addLogger(FileLogger& logger);

        void setFileLoggingEnabled(bool enabled) override;
        bool fileLoggingEnabled() const override;

    private:
        bool enabled_;
        std::vector<FileLogger*> loggers_;
    };
}

#endif //CLEANGRADUATOR_FILELOGGINGCONTROLLER_H
