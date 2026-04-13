#ifndef CLEANGRADUATOR_FILELOGGINGCONTROL_H
#define CLEANGRADUATOR_FILELOGGINGCONTROL_H

#include <atomic>

#include "application/ports/logging/IFileLoggingControl.h"

namespace infra::logging {
    class FileLoggingControl final : public application::ports::IFileLoggingControl {
    public:
        explicit FileLoggingControl(bool enabled);
        ~FileLoggingControl() override = default;

        void setFileLoggingEnabled(bool enabled) override;
        bool isFileLoggingEnabled() const override;

    private:
        std::atomic_bool enabled_;
    };
}

#endif // CLEANGRADUATOR_FILELOGGINGCONTROL_H
