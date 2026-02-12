#ifndef CLEANGRADUATOR_VIDEOSOURCENOTIFIER_H
#define CLEANGRADUATOR_VIDEOSOURCENOTIFIER_H
#include <algorithm>
#include <vector>

namespace domain::common {
    struct VideoSourceError;
    struct VideoFramePacket;
}

namespace domain::ports {
    struct IVideoSourceObserver;
}

namespace infra::camera::detail {
    class VideoSourceNotifier {
    public:
        void addObserver(domain::ports::IVideoSourceObserver& observer);
        void removeObserver(domain::ports::IVideoSourceObserver& observer);
        void notifyFrame(const domain::common::VideoFramePacket& frame);
        void notifyOpened();
        void notifyOpenFailed(const domain::common::VideoSourceError& error);
        void notifyClosed(const domain::common::VideoSourceError& error);

    private:
        std::vector<domain::ports::IVideoSourceObserver*> observers_;
    };
}


#endif //CLEANGRADUATOR_VIDEOSOURCENOTIFIER_H