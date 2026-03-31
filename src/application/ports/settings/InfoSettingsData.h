#ifndef CLEANGRADUATOR_INFOSETTINGSDATA_H
#define CLEANGRADUATOR_INFOSETTINGSDATA_H

namespace application::ports {
    struct InfoSettingsData {
        int displacement_idx{0};
        int gauge_idx{0};
        int precision_idx{0};
        int pressure_unit_idx{0};
        int printer_idx{0};
        bool ku_enabled{false};
        bool centered_mark_enabled{false};
        float max_center_deviation_deg{0.9F};
    };
}

#endif //CLEANGRADUATOR_INFOSETTINGSDATA_H
