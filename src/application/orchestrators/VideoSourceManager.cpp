#include "VideoSourceManager.h"

#include <set>

#include "domain/ports/inbound/IVideoSource.h"

using namespace application::orchestrators;

VideoSourceManager::VideoSourceManager(ports::IVideoAngleSourcesStorage &storage)
    : storage_(storage)
{
}

void VideoSourceManager::open(const std::vector<int>& ids)
{
    std::set new_ids(ids.begin(), ids.end());
    std::set current_ids(opened_.begin(), opened_.end());

    // 1️⃣ Закрыть те, которых больше нет
    for (int id : current_ids)
    {
        if (new_ids.count(id) == 0)
        {
            if (auto vs = storage_.at(id))
                vs->video_source.close();
        }
    }

    opened_.clear();

    // 2️⃣Открыть новые
    for (int id : new_ids)
    {
        auto vs = storage_.at(id);
        if (!vs) continue;

        if (current_ids.count(id) == 0)
        {
            if (!vs->video_source.open())
                continue;
        }

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
