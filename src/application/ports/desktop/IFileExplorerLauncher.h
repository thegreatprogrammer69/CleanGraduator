#ifndef CLEANGRADUATOR_IFILEEXPLORERLAUNCHER_H
#define CLEANGRADUATOR_IFILEEXPLORERLAUNCHER_H

#include <filesystem>
#include <string>

namespace application::ports {

class IFileExplorerLauncher {
public:
    struct Result {
        bool success;
        std::string error;
    };

    virtual ~IFileExplorerLauncher() = default;
    virtual Result showInExplorer(const std::filesystem::path& path) = 0;
};

}

#endif //CLEANGRADUATOR_IFILEEXPLORERLAUNCHER_H
