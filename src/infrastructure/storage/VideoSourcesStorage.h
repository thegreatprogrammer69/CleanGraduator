#ifndef CLEANGRADUATOR_VIDEOSOURCESTORAGE_H
#define CLEANGRADUATOR_VIDEOSOURCESTORAGE_H

#include <vector>
#include <optional>
#include <algorithm>

#include "../../application/ports/outbound/video/IVideoSourcesStorage.h"

namespace infra::storage {

    class VideoSourcesStorage : public application::ports::IVideoSourcesStorage {
    public:
        explicit VideoSourcesStorage();
        ~VideoSourcesStorage() override = default;

        void add(application::models::VideoSource source);
        std::vector<application::models::VideoSource> all() override;
        std::optional<application::models::VideoSource> at(int id) const override;

    private:
        std::vector<application::models::VideoSource> sources_;
    };

}

#endif //CLEANGRADUATOR_VIDEOSOURCESTORAGE_H