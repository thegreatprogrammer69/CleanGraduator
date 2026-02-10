#ifndef CLEANGRADUATOR_CAMERASETTINGS_H
#define CLEANGRADUATOR_CAMERASETTINGS_H
#include "VideoSourceGridString.h"
#include "VideoSourceCrosshair.h"

namespace application::dto {
    struct VideoSourceGridSettings {
        VideoSourceGridString grid_string;
        VideoSourceCrosshair crosshair;
        bool open_cameras_at_startup = false;
        bool operator==(const VideoSourceGridSettings& o) const {
            return
                grid_string == o.grid_string &&
                crosshair == o.crosshair &&
                open_cameras_at_startup == o.open_cameras_at_startup;
        }
    };
}

#endif //CLEANGRADUATOR_CAMERASETTINGS_H