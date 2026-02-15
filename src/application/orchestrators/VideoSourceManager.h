#ifndef CLEANGRADUATOR_VIDEOSOURCEMANAGER_H
#define CLEANGRADUATOR_VIDEOSOURCEMANAGER_H
#include "../ports/outbound/video/IVideoSourcesStorage.h"


namespace application::orchestrators {
    class VideoSourceManager {
    public:
        explicit VideoSourceManager(ports::IVideoSourcesStorage& storage);

        void open(const std::vector<int>& ids);
        void openAll();
        void closeAll();

        const std::vector<int>& opened() const;

    private:
        ports::IVideoSourcesStorage& storage_;
        std::vector<int> opened_;
    };
}



#endif //CLEANGRADUATOR_VIDEOSOURCEMANAGER_H