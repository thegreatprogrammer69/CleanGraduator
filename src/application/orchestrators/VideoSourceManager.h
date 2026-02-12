#ifndef CLEANGRADUATOR_VIDEOSOURCEMANAGER_H
#define CLEANGRADUATOR_VIDEOSOURCEMANAGER_H
#include "application/ports/outbound/IVideoSourceStorage.h"


namespace application::services {
    class VideoSourceManager {
    public:
        explicit VideoSourceManager(ports::IVideoSourceStorage& storage);

        void open(const std::vector<int>& ids);
        void openAll();
        void closeAll();

        const std::vector<int>& opened() const;

    private:
        ports::IVideoSourceStorage& storage_;
        std::vector<int> opened_;
    };
}



#endif //CLEANGRADUATOR_VIDEOSOURCEMANAGER_H