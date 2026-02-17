#ifndef CLEANGRADUATOR_INFOSETTINGSDATA_H
#define CLEANGRADUATOR_INFOSETTINGSDATA_H

namespace application::ports {
    struct InfoSettingsData {
        int displacement_idx{0};
        int gauge_idx{0};
        int precision_idx{0};
        int pressure_unit_idx{0};
        int printer_idx{0};
    };
}

#endif //CLEANGRADUATOR_INFOSETTINGSDATA_H