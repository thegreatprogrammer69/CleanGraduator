#ifndef CLEANGRADUATOR_IVIDEOSOURCESTORAGE_H
#define CLEANGRADUATOR_IVIDEOSOURCESTORAGE_H
#include <optional>
#include <vector>

#include "application/common/video/VideoSource.h"
#include "domain/core/angle/SourceId.h"

namespace application::ports {
    struct IVideoAngleSourcesStorage {
        virtual std::vector<models::VideoAngleSource> all() = 0;
        virtual std::optional<models::VideoAngleSource> at(domain::common::SourceId id) const = 0;
        virtual ~IVideoAngleSourcesStorage() = default;
    };
}

#endif //CLEANGRADUATOR_IVIDEOSOURCESTORAGE_H