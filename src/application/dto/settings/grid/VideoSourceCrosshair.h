#ifndef CLEANGRADUATOR_CAMERAGRIDCROSSHAIR_H
#define CLEANGRADUATOR_CAMERAGRIDCROSSHAIR_H
#include "VideoSourceCrosshairColor.h"

namespace application::dto {
    struct VideoSourceCrosshair {
        bool visible = false;
        float radius = 0.1;
        VideoSourceCrosshairColor color{};
        bool operator==(const VideoSourceCrosshair& o) const {
            return this->visible == o.visible && this->radius == o.radius && this->color == o.color;
        }
    };
}

#endif //CLEANGRADUATOR_CAMERAGRIDCROSSHAIR_H