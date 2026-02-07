#ifndef CLEANGRADUATOR_VIDEOCONFIG_H
#define CLEANGRADUATOR_VIDEOCONFIG_H
#include <string>

namespace infra::camera {

struct CameraConfig {
    std::string source;   // например "/dev/video0" в linux или "0" в windows.
    int width  = 640;
    int height = 480;
    int fps    = 30;
};

}

#endif //CLEANGRADUATOR_VIDEOCONFIG_H