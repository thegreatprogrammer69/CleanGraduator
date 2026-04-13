#include "FileLoggingController.h"

#include "FileLogger.h"

using namespace infra::logging;

FileLoggingController::FileLoggingController(bool enabled)
    : enabled_(enabled)
{
}

void FileLoggingController::addLogger(FileLogger& logger)
{
    logger.setEnabled(enabled_);
    loggers_.push_back(&logger);
}

void FileLoggingController::setFileLoggingEnabled(bool enabled)
{
    enabled_ = enabled;

    for (auto* logger : loggers_) {
        logger->setEnabled(enabled_);
    }
}

bool FileLoggingController::fileLoggingEnabled() const
{
    return enabled_;
}
