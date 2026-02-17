#ifndef CLEANGRADUATOR_VIDEOSOURCEMANAGER_H
#define CLEANGRADUATOR_VIDEOSOURCEMANAGER_H
#include "application/ports/video/IVideoAngleSourcesStorage.h"



namespace application::orchestrators {
    using domain::common::AngleSourceId;
    class VideoSourceManager {
    public:
        explicit VideoSourceManager(ports::IVideoAngleSourcesStorage& storage);

        void open(const std::vector<AngleSourceId>& ids);
        void openAll();
        void closeAll();

        const std::vector<AngleSourceId>& opened() const;

    private:
        ports::IVideoAngleSourcesStorage& storage_;
        std::vector<AngleSourceId> opened_;
    };
}



#endif //CLEANGRADUATOR_VIDEOSOURCEMANAGER_H