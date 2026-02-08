#ifndef CLEANGRADUATOR_APPCONFIG_H
#define CLEANGRADUATOR_APPCONFIG_H
#include <optional>

#include "infrastructure/camera/linux/v4l/V4LCameraConfig.h"


struct AppConfig {
    std::optional<V4LCameraConfig> v4_l_camera_config;
};

#endif //CLEANGRADUATOR_APPCONFIG_H