#ifndef CLEANGRADUATOR_IINFOSETTINGSSTORAGE_H
#define CLEANGRADUATOR_IINFOSETTINGSSTORAGE_H

#include "ISettingsStorage.h"

namespace application::ports {
    struct InfoSettingsData {
        int displacement_idx{0};
        int gauge_idx{0};
        int precision_idx{0};
        int pressure_unit_idx{0};
        int printer_idx{0};
    };

    struct IInfoSettingsStorage : ISettingsStorage {
        virtual InfoSettingsData loadInfoSettings() = 0;
        virtual void saveInfoSettings(const InfoSettingsData& data) = 0;
        ~IInfoSettingsStorage() override = default;
    };
}

#endif //CLEANGRADUATOR_IINFOSETTINGSSTORAGE_H
