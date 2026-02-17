#include "VideoAngleSourcesStorage.h"

using namespace infra::storage;
using namespace application::models;
using namespace domain::common;

VideoAngleSourcesStorage::VideoAngleSourcesStorage() {
}

void VideoAngleSourcesStorage::add(VideoAngleSource source) {
    sources_.push_back(std::move(source));
}

std::vector<VideoAngleSource> VideoAngleSourcesStorage::all()
{
    return sources_;  // возврат копии — безопасно
}

std::optional<VideoAngleSource> VideoAngleSourcesStorage::at(AngleSourceId id) const
{
    auto it = std::find_if(
        sources_.begin(),
        sources_.end(),
        [id](const VideoAngleSource& src) {
            return src.id == id;
        }
    );

    if (it == sources_.end())
        return std::nullopt;

    return *it; // копия
}
