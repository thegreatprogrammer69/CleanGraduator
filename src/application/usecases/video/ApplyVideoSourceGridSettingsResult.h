#ifndef CLEANGRADUATOR_APPLYVIDEOSOURCEGRIDSETTINGSRESULT_H
#define CLEANGRADUATOR_APPLYVIDEOSOURCEGRIDSETTINGSRESULT_H
#include <string>
#include "application/dto/settings/grid/VideoSourceGridSettings.h"

namespace application::usecase {
    struct ApplyVideoSourceGridSettingsResult {
        std::string error;
        dto::VideoSourceGridSettings correct;
    };
}

#endif //CLEANGRADUATOR_APPLYVIDEOSOURCEGRIDSETTINGSRESULT_H