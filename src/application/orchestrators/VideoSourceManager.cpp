#include "VideoSourceManager.h"

#include "domain/ports/inbound/IVideoSource.h"

using namespace application::orchestrators;

VideoSourceManager::VideoSourceManager(ports::IVideoSourceStorage &storage)
    : storage_(storage)
{
}

void VideoSourceManager::open(const std::vector<int>& ids)
{
    closeAll();

    opened_.clear();

    for (int id : ids)
    {
        auto vs = storage_.at(id);
        if (!vs) continue;

        vs->video_source.open();
        opened_.push_back(id);
    }
}

void VideoSourceManager::openAll()
{
    closeAll();
    opened_.clear();

    for (auto& vs : storage_.all())
    {
        vs.video_source.open();
        opened_.push_back(vs.id);
    }
}

void VideoSourceManager::closeAll()
{
    for (int id : opened_)
    {
        auto vs = storage_.at(id);
        if (vs)
            vs->video_source.close();
    }

    opened_.clear();
}

const std::vector<int>& VideoSourceManager::opened() const
{
    return opened_;
}
