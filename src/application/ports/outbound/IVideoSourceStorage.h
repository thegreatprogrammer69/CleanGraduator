#ifndef CLEANGRADUATOR_IVIDEOSOURCESTORAGE_H
#define CLEANGRADUATOR_IVIDEOSOURCESTORAGE_H
#include <optional>
#include <vector>

#include "application/models/VideoSource.h"

namespace application::ports {
    struct IVideoSourceStorage {
        virtual std::vector<models::VideoSource> all() = 0;
        virtual std::optional<models::VideoSource> at(int id) const = 0;
        virtual ~IVideoSourceStorage() = default;
    };
}

#endif //CLEANGRADUATOR_IVIDEOSOURCESTORAGE_H