#include "VideoSourceViewModel.h"

#include "domain/events/IEventBus.h"
#include "domain/events/video/VideoSourceClosed.h"
#include "domain/events/video/VideoSourceOpened.h"
#include "domain/events/video/VideoSourceOpenFailed.h"
#include "domain/ports/inbound/IVideoSource.h"

using namespace mvvm;
using namespace domain::events;

VideoSourceViewModel::VideoSourceViewModel(domain::ports::IVideoSource& video_source, domain::events::IEventBus& event_bus)
    : video_source_(video_source)
    , event_bus_(event_bus)
{
    video_source_.addSink(*this);
    event_bus_.addListener(*this);
}

VideoSourceViewModel::~VideoSourceViewModel() {
    video_source_.removeSink(*this);
    event_bus_.removeListener(*this);
}

void VideoSourceViewModel::onVideoFrame(const domain::common::Timestamp &, domain::common::VideoFramePtr _frame) {
    frame.set(_frame);
}

void VideoSourceViewModel::onEvent(const Event *event) {

    if (event->type() == EventType::VideoSourceClosed) {
        is_opened.set(false);
    }
    else if (event->type() == EventType::VideoSourceOpened) {
        is_opened.set(true);
    }
    else if (const auto* e = dynamic_cast<const VideoSourceOpenFailed*>(event); e != nullptr){
        is_opened.set(false);
        error.set(e->reason);
    }
}
