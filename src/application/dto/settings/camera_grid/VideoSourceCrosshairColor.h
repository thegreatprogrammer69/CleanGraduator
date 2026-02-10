#ifndef CLEANGRADUATOR_CAMERAGRIDCROSSHAIRCOLOR_H
#define CLEANGRADUATOR_CAMERAGRIDCROSSHAIRCOLOR_H
#include <cstdint>

namespace application::dto {
    struct VideoSourceCrosshairColor {
        std::uint32_t color1{0xFFFFFFFF};
        std::uint32_t color2{0x000000FF};
        bool operator==(const VideoSourceCrosshairColor& o) const {
            return this->color1 == o.color1 && this->color2 == o.color2;
        }
    };
}

#endif //CLEANGRADUATOR_CAMERAGRIDCROSSHAIRCOLOR_H