#include "FileLogger.h"

namespace infra::logging {

FileLogger::FileLogger(const std::string &filePath): file_(filePath, std::ios::app) {}

void FileLogger::info(const std::string &msg) {
    log("INFO", msg);
}

void FileLogger::warn(const std::string &msg) {
    log("WARN", msg);
}

void FileLogger::error(const std::string &msg) {
    log("ERROR", msg);
}

void FileLogger::log(const std::string &level, const std::string &msg) {
    std::lock_guard<std::mutex> lock(mutex_);
    file_ << "[" << level << "] " << msg << std::endl;
}

}
