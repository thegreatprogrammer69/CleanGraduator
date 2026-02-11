#ifndef CLEANGRADUATOR_VIDEOSOURCEGRIDSERVICEAPPLYRESULT_H
#define CLEANGRADUATOR_VIDEOSOURCEGRIDSERVICEAPPLYRESULT_H
#include "application/dto/settings/grid/VideoSourceGridSettings.h"
#include <string>

namespace application::services {
    struct VideoSourceGridServiceApplyResult {
        dto::VideoSourceGridSettings correct_settings;
        std::string error;
    };

    using VideoSourceGridServiceOpenResult = VideoSourceGridServiceApplyResult;
}

#endif //CLEANGRADUATOR_VIDEOSOURCEGRIDSERVICEAPPLYRESULT_H