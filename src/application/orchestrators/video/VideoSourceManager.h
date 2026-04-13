#ifndef CLEANGRADUATOR_VIDEOSOURCEMANAGER_H
#define CLEANGRADUATOR_VIDEOSOURCEMANAGER_H
#include "application/ports/video/IVideoAngleSourcesStorage.h"



namespace application::orchestrators {
    using domain::common::SourceId;
    class VideoSourceManager {
    public:
        explicit VideoSourceManager(ports::IVideoAngleSourcesStorage& storage);

        void open(const std::vector<SourceId>& ids);
        void openAll();
        void closeAll();

        std::vector<SourceId> available();
        const std::vector<SourceId>& opened() const;

    private:
        ports::IVideoAngleSourcesStorage& storage_;
        std::vector<SourceId> opened_;
    };
}



#endif //CLEANGRADUATOR_VIDEOSOURCEMANAGER_H
