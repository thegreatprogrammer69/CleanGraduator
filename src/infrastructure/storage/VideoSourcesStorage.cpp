#include "VideoSourcesStorage.h"

using namespace infra::storage;
using namespace application::models;

VideoSourcesStorage::VideoSourcesStorage() {
}

void VideoSourcesStorage::add(VideoSource source) {
    sources_.push_back(std::move(source));
}

std::vector<VideoSource> VideoSourcesStorage::all()
{
    return sources_;  // возврат копии — безопасно
}

std::optional<VideoSource> VideoSourcesStorage::at(int id) const
{
    auto it = std::find_if(
        sources_.begin(),
        sources_.end(),
        [id](const VideoSource& src) {
            return src.id == id;
        }
    );

    if (it == sources_.end())
        return std::nullopt;

    return *it; // копия
}
