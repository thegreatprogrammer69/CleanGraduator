#ifndef CLEANGRADUATOR_OPENSELECTEDCAMERAS_H
#define CLEANGRADUATOR_OPENSELECTEDCAMERAS_H
#include <string>
#include <vector>

namespace application::orchestrators {
    class VideoSourceManager;
}

namespace application::usecase {
    class OpenSelectedCameras {
    public:
        explicit OpenSelectedCameras(orchestrators::VideoSourceManager& manager)
            : manager_(manager) {}

        std::vector<int> execute(const std::vector<int>& indexes);

    private:
        orchestrators::VideoSourceManager& manager_;
    };
}


#endif //CLEANGRADUATOR_OPENSELECTEDCAMERAS_H