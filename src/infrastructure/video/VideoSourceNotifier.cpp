#include "VideoSourceNotifier.h"
#include "domain/ports/video/IVideoSourceObserver.h"
#include "domain/ports/video/IVideoSink.h"

using namespace infra::camera::detail;
using namespace domain::ports;
using namespace domain::common;

void VideoSourceNotifier::addObserver(IVideoSourceObserver& o)
{
    observers_.add(o);
}

void VideoSourceNotifier::removeObserver(IVideoSourceObserver& o)
{
    observers_.remove(o);
}

void VideoSourceNotifier::addSink(IVideoSink& s)
{
    sinks_.add(s);
}

void VideoSourceNotifier::removeSink(IVideoSink& s)
{
    sinks_.remove(s);
}

void VideoSourceNotifier::notifyFrame(const VideoFramePacket& frame)
{
    sinks_.notify([&frame](IVideoSink& s) {
        s.onVideoFrame(frame);
    });
}

void VideoSourceNotifier::notifyEvent(const VideoSourceEvent& event)
{
    observers_.notify([&event](IVideoSourceObserver& o) {
        o.onVideoSourceEvent(event);
    });
}