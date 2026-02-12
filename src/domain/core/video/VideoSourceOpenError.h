#ifndef CLEANGRADUATOR_VIDEOSOURCEOPENERROR_H
#define CLEANGRADUATOR_VIDEOSOURCEOPENERROR_H
#include <string>

namespace domain::common {
    struct VideoSourceOpenError {
        std::string message;   // опционально
    };
}

#endif //CLEANGRADUATOR_VIDEOSOURCEOPENERROR_H