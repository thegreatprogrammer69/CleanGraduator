#ifndef CLEANGRADUATOR_OPENALLCAMERAS_H
#define CLEANGRADUATOR_OPENALLCAMERAS_H

#include <vector>

namespace application::orchestrators {
    class VideoSourceManager;
}

namespace application::usecase {

    class OpenAllCameras {
    public:
        explicit OpenAllCameras(orchestrators::VideoSourceManager& manager)
            : manager_(manager) {}

        std::vector<int> execute();

    private:
        orchestrators::VideoSourceManager& manager_;
    };

}

#endif // CLEANGRADUATOR_OPENALLCAMERAS_H
