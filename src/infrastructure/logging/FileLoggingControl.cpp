#include "FileLoggingControl.h"

using infra::logging::FileLoggingControl;

FileLoggingControl::FileLoggingControl(const bool enabled)
    : enabled_(enabled) {
}

void FileLoggingControl::setFileLoggingEnabled(const bool enabled) {
    enabled_.store(enabled, std::memory_order_relaxed);
}

bool FileLoggingControl::isFileLoggingEnabled() const {
    return enabled_.load(std::memory_order_relaxed);
}
