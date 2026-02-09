#ifndef CLEANGRADUATOR_FRAMECROSSHAIR_H
#define CLEANGRADUATOR_FRAMECROSSHAIR_H
#include <string>
#include <cstdint>

namespace application::dto {
    struct FrameCrosshair {
        bool visible = false;
        float radius = 0.1;
        std::uint32_t color = 0xFFFFFFFF; // например, белый ARGB
    };
}

#endif //CLEANGRADUATOR_FRAMECROSSHAIR_H