#ifndef CLEANGRADUATOR_VIDEOSOURCEGRIDSETTINGS_H
#define CLEANGRADUATOR_VIDEOSOURCEGRIDSETTINGS_H
#include "VideoSourceCrosshair.h"
#include "VideoSourceGridString.h"

namespace application::dto {
    struct VideoSourceGridSettings {
        VideoSourceCrosshair crosshair{};
        VideoSourceGridString string{};
        bool open_cameras_at_startup{};
    };
}

#endif //CLEANGRADUATOR_VIDEOSOURCEGRIDSETTINGS_H