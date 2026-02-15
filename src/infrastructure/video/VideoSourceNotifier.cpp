#include "VideoSourceNotifier.h"

#include "../../domain/ports/video/IVideoSourceObserver.h"

using namespace infra::camera::detail;
using namespace domain::ports;
using namespace domain::common;

void VideoSourceNotifier::addObserver(IVideoSourceObserver &observer) {
    observers_.push_back(&observer);
}

void VideoSourceNotifier::removeObserver(IVideoSourceObserver &observer) {
    observers_.erase(
        std::remove(observers_.begin(), observers_.end(), &observer),
        observers_.end()
    );
}

void VideoSourceNotifier::notifyFrame(const VideoFramePacket &frame) {
    for (auto* o : observers_)
        o->onVideoFrame(frame);
}

void VideoSourceNotifier::notifyOpened() {
    for (auto* o : observers_)
        o->onVideoSourceOpened();
}

void VideoSourceNotifier::notifyOpenFailed(const VideoSourceError &error) {
    for (auto* o : observers_)
        o->onVideoSourceOpenFailed(error);
}

void VideoSourceNotifier::notifyClosed(const VideoSourceError& error) {
    for (auto* o : observers_)
        o->onVideoSourceClosed(error);
}
