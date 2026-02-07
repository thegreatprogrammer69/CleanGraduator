#include "ConsoleLogger.h"

namespace infra::logging {

void ConsoleLogger::info(const std::string &msg) {
    std::cout << "[INFO] " << msg << std::endl;
}

void ConsoleLogger::warn(const std::string &msg) {
    std::cout << "[WARN] " << msg << std::endl;
}

void ConsoleLogger::error(const std::string &msg) {
    std::cerr << "[ERROR] " << msg << std::endl;
}

}
