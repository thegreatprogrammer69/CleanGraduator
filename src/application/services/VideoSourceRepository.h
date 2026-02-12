#ifndef CLEANGRADUATOR_VIDEOSOURCEREPOSITORY_H
#define CLEANGRADUATOR_VIDEOSOURCEREPOSITORY_H
#include <optional>
#include <vector>

#include "domain/ports/inbound/IVideoSource.h"

namespace application::services {

    struct VideoSource {
        int id;
        domain::ports::IVideoSource& source;
    };

    class VideoSourceRepository {
    public:
        explicit VideoSourceRepository(std::vector<VideoSource> sources);

        std::vector<VideoSource> all() const;
        std::optional<VideoSource> at(int id) const;
    private:
        std::vector<VideoSource> sources_;
    };
}

#endif //CLEANGRADUATOR_VIDEOSOURCEREPOSITORY_H