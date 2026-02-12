#ifndef CLEANGRADUATOR_OPENSELECTEDCAMERAS_H
#define CLEANGRADUATOR_OPENSELECTEDCAMERAS_H
#include <string>

namespace application::services {
    class VideoSourceManager;
}

namespace application::usecase {
    class OpenSelectedCameras {
    public:
        explicit OpenSelectedCameras(services::VideoSourceManager& manager)
            : manager_(manager) {}

        void execute(const std::string& input);

    private:
        services::VideoSourceManager& manager_;
    };
}


#endif //CLEANGRADUATOR_OPENSELECTEDCAMERAS_H