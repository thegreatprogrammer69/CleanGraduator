#ifndef CLEANGRADUATOR_VIDEOSOURCESTORAGE_H
#define CLEANGRADUATOR_VIDEOSOURCESTORAGE_H

#include <vector>
#include <optional>
#include <algorithm>

#include "../../application/ports/outbound/video/IVideoAngleSourcesStorage.h"

namespace infra::storage {

    class VideoAngleSourcesStorage : public application::ports::IVideoAngleSourcesStorage {
    public:
        explicit VideoAngleSourcesStorage();
        ~VideoAngleSourcesStorage() override = default;

        void add(application::models::VideoAngleSource source);
        std::vector<application::models::VideoAngleSource> all() override;
        std::optional<application::models::VideoAngleSource> at(int id) const override;

    private:
        std::vector<application::models::VideoAngleSource> sources_;
    };

}

#endif //CLEANGRADUATOR_VIDEOSOURCESTORAGE_H