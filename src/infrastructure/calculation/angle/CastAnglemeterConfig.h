#ifndef CLEANGRADUATOR_CASTANGLEMETERCONFIG_H
#define CLEANGRADUATOR_CASTANGLEMETERCONFIG_H
#include <vector>

namespace infra::calc {
    struct CastAnglemeterConfig {
        int bright_lim = 150;
        int max_pairs = 6;
        int scan_step = 2;
        std::vector<int> offsets = {0, -8, +8, -16, +16, -24, +24, -36, +36, -48, +48, -56, +56, -64, +64, -82, +82, -96, +96};
    };
}

#endif //CLEANGRADUATOR_CASTANGLEMETERCONFIG_H