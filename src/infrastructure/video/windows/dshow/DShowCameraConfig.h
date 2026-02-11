#ifndef CLEANGRADUATOR_DSHOWCAMERACONFIG_H
#define CLEANGRADUATOR_DSHOWCAMERACONFIG_H
#include <string>

namespace infra::camera {
    struct DShowCameraConfig {
        int index;
        int width  = 640;
        int height = 480;
        int fps    = 30;
    };
}


#endif //CLEANGRADUATOR_DSHOWCAMERACONFIG_H