#ifndef CLEANGRADUATOR_FAKEANGLESOURCEFROMVIDEOCONFIG_H
#define CLEANGRADUATOR_FAKEANGLESOURCEFROMVIDEOCONFIG_H
#include <cstdint>

namespace infra::angle {
    struct FakeAngleSourceFromVideoConfig {
        double from_deg{0.0};
        double to_deg{360.0};
        std::uint64_t duration_ms{10000}; // время на ход в одну сторону
    };
}

#endif //CLEANGRADUATOR_FAKEANGLESOURCEFROMVIDEOCONFIG_H