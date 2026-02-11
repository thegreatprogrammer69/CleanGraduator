#ifndef CLEANGRADUATOR_VIDEOSOURCEREPOSITORY_H
#define CLEANGRADUATOR_VIDEOSOURCEREPOSITORY_H

#include <memory>
#include <string>
#include <vector>
#include "infrastructure/video/VideoSourcePorts.h"

namespace domain::ports {
    struct IVideoSource;
}

namespace infra::repo {
    class VideoSourceFactory final {
    public:
        explicit VideoSourceFactory(const std::string& ini_path, camera::VideoSourcePorts ports);
        ~VideoSourceFactory();

        std::vector<std::unique_ptr<domain::ports::IVideoSource>> load();

    private:
        std::string ini_path_;
        camera::VideoSourcePorts ports_;

    };
}

#endif //CLEANGRADUATOR_VIDEOSOURCEREPOSITORY_H