#ifndef CLEANGRADUATOR_OPENSELECTEDCAMERAS_H
#define CLEANGRADUATOR_OPENSELECTEDCAMERAS_H
#include <string>

namespace application::orchestrators {
    class VideoSourceManager;
}

namespace application::usecase {
    class OpenSelectedCameras {
    public:
        explicit OpenSelectedCameras(orchestrators::VideoSourceManager& manager)
            : manager_(manager) {}

        void execute(const std::string& input);

    private:
        orchestrators::VideoSourceManager& manager_;
    };
}


#endif //CLEANGRADUATOR_OPENSELECTEDCAMERAS_H