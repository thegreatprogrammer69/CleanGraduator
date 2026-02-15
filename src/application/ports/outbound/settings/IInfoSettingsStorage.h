#ifndef CLEANGRADUATOR_IINFOSETTINGSSTORAGE_H
#define CLEANGRADUATOR_IINFOSETTINGSSTORAGE_H

namespace application::ports {
    struct InfoSettingsData {
        int displacement_idx{0};
        int gauge_idx{0};
        int precision_idx{0};
        int pressure_unit_idx{0};
        int printer_idx{0};
    };

    struct IInfoSettingsStorage  {
        virtual InfoSettingsData loadInfoSettings() = 0;
        virtual void saveInfoSettings(const InfoSettingsData& data) = 0;
        virtual ~IInfoSettingsStorage() = default;
    };
}

#endif //CLEANGRADUATOR_IINFOSETTINGSSTORAGE_H
