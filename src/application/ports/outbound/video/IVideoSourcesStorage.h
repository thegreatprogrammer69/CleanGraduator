#ifndef CLEANGRADUATOR_IVIDEOSOURCESTORAGE_H
#define CLEANGRADUATOR_IVIDEOSOURCESTORAGE_H
#include <optional>
#include <vector>

#include "../../../models/video/VideoSource.h"

namespace application::ports {
    struct IVideoSourcesStorage {
        virtual std::vector<models::VideoSource> all() = 0;
        virtual std::optional<models::VideoSource> at(int id) const = 0;
        virtual ~IVideoSourcesStorage() = default;
    };
}

#endif //CLEANGRADUATOR_IVIDEOSOURCESTORAGE_H