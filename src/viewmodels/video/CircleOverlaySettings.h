#ifndef CLEANGRADUATOR_CIRCLEOVERLAYSETTINGS_H
#define CLEANGRADUATOR_CIRCLEOVERLAYSETTINGS_H

#include <cstdint>

namespace mvvm {

struct CircleOverlaySettings {
    int diameter_percent = 30;
    std::uint32_t color1 = 0xFFFFFFFF;
    std::uint32_t color2 = 0x000000FF;

    bool operator==(const CircleOverlaySettings& other) const noexcept {
        return diameter_percent == other.diameter_percent
            && color1 == other.color1
            && color2 == other.color2;
    }
};

}

#endif // CLEANGRADUATOR_CIRCLEOVERLAYSETTINGS_H
