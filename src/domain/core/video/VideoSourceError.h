#ifndef CLEANGRADUATOR_VIDEOSOURCEOPENERROR_H
#define CLEANGRADUATOR_VIDEOSOURCEOPENERROR_H
#include <string>

namespace domain::common {
    struct VideoSourceError {
        std::string reason;
    };
}

#endif //CLEANGRADUATOR_VIDEOSOURCEOPENERROR_H