#ifndef CLEANGRADUATOR_CROSSHAIROVERLAYCONFIG_H
#define CLEANGRADUATOR_CROSSHAIROVERLAYCONFIG_H
#include <cstdint>

namespace infra::overlay {
    struct CrosshairVideoOverlayConfig {
        bool visible = true;
        float radius = 0.15;
        std::uint32_t color1 = 0xFFFFFFFF;
        std::uint32_t color2 = 0x000000FF;
    };
}

#endif //CLEANGRADUATOR_CROSSHAIROVERLAYCONFIG_H