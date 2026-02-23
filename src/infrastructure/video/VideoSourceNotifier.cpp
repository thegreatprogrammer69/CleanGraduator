#include "VideoSourceNotifier.h"
#include "../../domain/ports/video/IVideoSourceObserver.h"

using namespace infra::camera::detail;
using namespace domain::ports;
using namespace domain::common;

void VideoSourceNotifier::addObserver(IVideoSourceObserver& observer) {
    std::lock_guard<std::mutex> lock(mutex_);
    observers_.push_back(&observer);
}

void VideoSourceNotifier::removeObserver(IVideoSourceObserver& observer) {
    std::lock_guard<std::mutex> lock(mutex_);
    observers_.erase(
        std::remove(observers_.begin(), observers_.end(), &observer),
        observers_.end()
    );
}

void VideoSourceNotifier::notifyFrame(const VideoFramePacket& frame) {
    std::vector<IVideoSourceObserver*> copy;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        copy = observers_;
    }

    for (auto* o : copy)
        o->onVideoFrame(frame);
}

void VideoSourceNotifier::notifyOpened() {
    std::vector<IVideoSourceObserver*> copy;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        copy = observers_;
    }

    for (auto* o : copy)
        o->onVideoSourceOpened();
}

void VideoSourceNotifier::notifyFailed(const VideoSourceError& error) {
    std::vector<IVideoSourceObserver*> copy;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        copy = observers_;
    }

    for (auto* o : copy)
        o->onVideoSourceFailed(error);
}

void VideoSourceNotifier::notifyClosed() {
    std::vector<IVideoSourceObserver*> copy;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        copy = observers_;
    }

    for (auto* o : copy)
        o->onVideoSourceClosed();
}