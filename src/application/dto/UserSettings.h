#ifndef CLEANGRADUATOR_UISETTINGSDTO_H
#define CLEANGRADUATOR_UISETTINGSDTO_H
#include "settings/camera_grid/VideoSourceGridSettings.h"

namespace application::dto {

    struct UserSettings {
        VideoSourceGridSettings camera_grid;

        bool operator==(const UserSettings& o) const {
            return camera_grid == o.camera_grid;
        }
    };

} // namespace application::dto

#endif //CLEANGRADUATOR_UISETTINGSDTO_H