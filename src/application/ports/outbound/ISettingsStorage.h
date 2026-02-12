#ifndef CLEANGRADUATOR_ISETTINGSSTORAGE_H
#define CLEANGRADUATOR_ISETTINGSSTORAGE_H
#include "application/settings/video/VideoGridSettings.h"

namespace application::ports {
    struct ISettingsStorage {

        virtual models::VideoGridSettings loadVideoGridSettings() = 0;
        virtual void saveVideoGridSettings(const models::VideoGridSettings&) = 0;

        virtual ~ISettingsStorage() = 0;

    };
}

#endif //CLEANGRADUATOR_ISETTINGSSTORAGE_H