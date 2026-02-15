#ifndef CLEANGRADUATOR_VIDEOSOURCEMANAGER_H
#define CLEANGRADUATOR_VIDEOSOURCEMANAGER_H
#include "../ports/outbound/video/IVideoAngleSourcesStorage.h"


namespace application::orchestrators {
    class VideoSourceManager {
    public:
        explicit VideoSourceManager(ports::IVideoAngleSourcesStorage& storage);

        void open(const std::vector<int>& ids);
        void openAll();
        void closeAll();

        const std::vector<int>& opened() const;

    private:
        ports::IVideoAngleSourcesStorage& storage_;
        std::vector<int> opened_;
    };
}



#endif //CLEANGRADUATOR_VIDEOSOURCEMANAGER_H