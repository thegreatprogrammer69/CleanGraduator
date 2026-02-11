#ifndef CLEANGRADUATOR_V4LCAMERACONFIG_H
#define CLEANGRADUATOR_V4LCAMERACONFIG_H
#include <string>

namespace infra::camera {
    struct V4LCameraConfig {
        std::string source;   // например "/dev/video0"
        int width  = 640;
        int height = 480;
        int fps    = 30;
    };
}


#endif //CLEANGRADUATOR_V4LCAMERACONFIG_H