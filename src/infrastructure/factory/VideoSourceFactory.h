#ifndef CLEANGRADUATOR_VIDEOSOURCEREPOSITORY_H
#define CLEANGRADUATOR_VIDEOSOURCEREPOSITORY_H

#include <memory>
#include <string>
#include <vector>
#include "infrastructure/video/VideoSourcePorts.h"

namespace application::ports {
    struct ILoggerFactory;
}

namespace domain::ports {
    struct IVideoSource;
}

namespace infra::repo {
    class VideoSourceFactory final {
    public:
        explicit VideoSourceFactory(const std::string& ini_path, domain::ports::IClock& clock,
            application::ports::ILoggerFactory& logger_factory);
        ~VideoSourceFactory();

        std::vector<std::unique_ptr<domain::ports::IVideoSource>> load();

    private:
        std::string ini_path_;
        domain::ports::IClock& clock_;
        application::ports::ILoggerFactory& logger_factory_;

    };
}

#endif //CLEANGRADUATOR_VIDEOSOURCEREPOSITORY_H