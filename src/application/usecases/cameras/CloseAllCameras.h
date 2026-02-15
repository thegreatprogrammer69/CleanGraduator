#ifndef CLEANGRADUATOR_CLOSEALLCAMERAS_H
#define CLEANGRADUATOR_CLOSEALLCAMERAS_H

#include <vector>

namespace application::orchestrators {
    class VideoSourceManager;
}

namespace application::usecase {

    class CloseAllCameras {
    public:
        explicit CloseAllCameras(orchestrators::VideoSourceManager& manager)
            : manager_(manager) {}

        std::vector<int> execute();

    private:
        orchestrators::VideoSourceManager& manager_;
    };

}

#endif // CLEANGRADUATOR_CLOSEALLCAMERAS_H
