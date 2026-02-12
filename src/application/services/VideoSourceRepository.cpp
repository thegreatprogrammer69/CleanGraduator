#include "VideoSourceRepository.h"

using namespace application::services;

VideoSourceRepository::VideoSourceRepository(std::vector<VideoSource> sources)
    : sources_(std::move(sources)) { }

std::vector<VideoSource> VideoSourceRepository::all() const {
    return sources_;
}

std::optional<VideoSource> VideoSourceRepository::at(int id) const {
    for (const auto& source : sources_)
        if (source.id == id)
            return source;
    return std::nullopt;
}
