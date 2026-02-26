#ifndef CLEANGRADUATOR_VIDEOSOURCENOTIFIER_H
#define CLEANGRADUATOR_VIDEOSOURCENOTIFIER_H
#include "infrastructure/utils/thread/ThreadSafeObserverList.h"

namespace domain::ports {
    class IVideoSourceObserver;
    class IVideoSink;
}

namespace domain::common {
    struct VideoFramePacket;
    struct VideoSourceEvent;
}


namespace infra::camera::detail {

    class VideoSourceNotifier {
    public:
        void addObserver(domain::ports::IVideoSourceObserver& o);
        void removeObserver(domain::ports::IVideoSourceObserver& o);

        void addSink(domain::ports::IVideoSink& s);
        void removeSink(domain::ports::IVideoSink& s);

        void notifyFrame(const domain::common::VideoFramePacket& frame);
        void notifyEvent(const domain::common::VideoSourceEvent& event);

    private:
        ThreadSafeObserverList<domain::ports::IVideoSourceObserver> observers_;
        ThreadSafeObserverList<domain::ports::IVideoSink> sinks_;
    };


}

#endif // CLEANGRADUATOR_VIDEOSOURCENOTIFIER_H