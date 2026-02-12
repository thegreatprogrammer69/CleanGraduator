#ifndef CLEANGRADUATOR_LAYOUTHINT_H
#define CLEANGRADUATOR_LAYOUTHINT_H

namespace application::models {
    struct LayoutHint {
        int row = -1;        // -1 = авто
        int column = 0;
        int columnSpan = 1;
    };
}

#endif //CLEANGRADUATOR_LAYOUTHINT_H