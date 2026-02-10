#ifndef CLEANGRADUATOR_ICAMERAGRIDCROSSHAIROBSERVER_H
#define CLEANGRADUATOR_ICAMERAGRIDCROSSHAIROBSERVER_H
#include "application/dto/settings/camera_grid/VideoSourceCrosshair.h"

namespace application::ports {
    struct IVideoSourceCrosshairListener {
        virtual void onCrosshairChanged(const dto::VideoSourceCrosshair&) = 0;
        virtual ~IVideoSourceCrosshairListener() = default;
    };
}

#endif //CLEANGRADUATOR_ICAMERAGRIDCROSSHAIROBSERVER_H