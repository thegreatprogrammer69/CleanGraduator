#ifndef CLEANGRADUATOR_IINFOSETTINGSSTORAGE_H
#define CLEANGRADUATOR_IINFOSETTINGSSTORAGE_H
#include "InfoSettingsData.h"

namespace application::ports {
    struct IInfoSettingsStorage  {
        virtual InfoSettingsData loadInfoSettings() = 0;
        virtual void saveInfoSettings(const InfoSettingsData& data) = 0;
        virtual ~IInfoSettingsStorage() = default;
    };
}

#endif //CLEANGRADUATOR_IINFOSETTINGSSTORAGE_H
