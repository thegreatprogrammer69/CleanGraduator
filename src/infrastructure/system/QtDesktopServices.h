#ifndef CLEANGRADUATOR_QTDESKTOPSERVICES_H
#define CLEANGRADUATOR_QTDESKTOPSERVICES_H

#include "application/ports/system/IDesktopServices.h"

namespace infra::system {
    class QtDesktopServices final : public application::ports::IDesktopServices {
    public:
        Result revealInFileManager(const std::filesystem::path& path) override;
    };
}

#endif //CLEANGRADUATOR_QTDESKTOPSERVICES_H
