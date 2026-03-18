#ifndef CLEANGRADUATOR_QTFILEEXPLORERLAUNCHER_H
#define CLEANGRADUATOR_QTFILEEXPLORERLAUNCHER_H

#include "application/ports/desktop/IFileExplorerLauncher.h"

namespace infra::desktop {

class QtFileExplorerLauncher final : public application::ports::IFileExplorerLauncher {
public:
    Result showInExplorer(const std::filesystem::path& path) override;
};

}

#endif //CLEANGRADUATOR_QTFILEEXPLORERLAUNCHER_H
