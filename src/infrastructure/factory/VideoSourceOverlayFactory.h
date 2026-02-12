#ifndef CLEANGRADUATOR_VIDEOSOURCEOVERLAYFACTORY_H
#define CLEANGRADUATOR_VIDEOSOURCEOVERLAYFACTORY_H


#include <memory>
#include <string>
#include <vector>

#include "domain/ports/inbound/IVideoSourceOverlay.h"
#include "infrastructure/overlay/VideoOverlayPorts.h"

namespace infra::repo {

    class VideoSourceOverlayFactory final {
    public:
        explicit VideoSourceOverlayFactory(const std::string& ini_path, overlay::VideoOverlayPorts ports);

        ~VideoSourceOverlayFactory();

        std::vector<std::unique_ptr<domain::ports::IVideoSourceOverlay>> load();

    private:
        std::string ini_path_;
        overlay::VideoOverlayPorts ports_;
    };

}

#endif //CLEANGRADUATOR_VIDEOSOURCEOVERLAYFACTORY_H