#ifndef CLEANGRADUATOR_IDESKTOPSERVICES_H
#define CLEANGRADUATOR_IDESKTOPSERVICES_H

#include <filesystem>
#include <string>

namespace application::ports {
    class IDesktopServices {
    public:
        struct Result {
            bool success;
            std::string error;
        };

        virtual ~IDesktopServices() = default;
        virtual Result revealInFileManager(const std::filesystem::path& path) = 0;
    };
}

#endif //CLEANGRADUATOR_IDESKTOPSERVICES_H
