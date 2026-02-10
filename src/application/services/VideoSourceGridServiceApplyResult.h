#ifndef CLEANGRADUATOR_VIDEOSOURCEGRIDSERVICEAPPLYRESULT_H
#define CLEANGRADUATOR_VIDEOSOURCEGRIDSERVICEAPPLYRESULT_H
#include "application/dto/settings/camera_grid/VideoSourceGridString.h"
#include <string>

namespace application::services {
    struct VideoSourceGridServiceApplyResult {
        dto::VideoSourceGridString correct_string;
        std::string error;
    };
}

#endif //CLEANGRADUATOR_VIDEOSOURCEGRIDSERVICEAPPLYRESULT_H