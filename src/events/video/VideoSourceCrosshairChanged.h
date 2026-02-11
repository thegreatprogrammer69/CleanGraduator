#ifndef CLEANGRADUATOR_VIDEOSOURCECROSSHAIRCHANGED_H
#define CLEANGRADUATOR_VIDEOSOURCECROSSHAIRCHANGED_H
#include "application/dto/settings/grid/VideoSourceCrosshair.h"
#include "domain/events/video/VideoEvent.h"

namespace application::events {
    struct VideoSourceCrosshairChanged : domain::events::VideoEvent {
        dto::VideoSourceCrosshair crosshair;
    };
}

#endif //CLEANGRADUATOR_VIDEOSOURCECROSSHAIRCHANGED_H
