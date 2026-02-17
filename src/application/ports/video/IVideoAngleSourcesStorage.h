#ifndef CLEANGRADUATOR_IVIDEOSOURCESTORAGE_H
#define CLEANGRADUATOR_IVIDEOSOURCESTORAGE_H
#include <optional>
#include <vector>

#include "application/models/video/VideoSource.h"
#include "domain/core/angle/AngleSourceId.h"

namespace application::ports {
    struct IVideoAngleSourcesStorage {
        virtual std::vector<models::VideoAngleSource> all() = 0;
        virtual std::optional<models::VideoAngleSource> at(domain::common::AngleSourceId id) const = 0;
        virtual ~IVideoAngleSourcesStorage() = default;
    };
}

#endif //CLEANGRADUATOR_IVIDEOSOURCESTORAGE_H