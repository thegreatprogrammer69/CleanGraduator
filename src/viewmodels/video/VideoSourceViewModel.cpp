#include "VideoSourceViewModel.h"

#include "domain/core/video/VideoFramePacket.h"
#include "domain/core/video/VideoSourceError.h"
#include "domain/ports/video/IVideoSource.h"

using namespace mvvm;

VideoSourceViewModel::VideoSourceViewModel(domain::ports::IVideoSource& video_source)
    : video_source_(video_source)
{
    video_source_.addObserver(*this);
}

VideoSourceViewModel::~VideoSourceViewModel() {
    video_source_.removeObserver(*this);
}

void VideoSourceViewModel::onVideoSourceOpened() {
    is_opened.set(true);
}

void VideoSourceViewModel::onVideoSourceFailed(const domain::common::VideoSourceError &err) {
    is_opened.set(false);
    error.set(err.reason);
}

void VideoSourceViewModel::onVideoSourceClosed() {
    is_opened.set(false);
}

void VideoSourceViewModel::onVideoFrame(const domain::common::VideoFramePacket& frame_packet) {
    frame.set(frame_packet.frame);
    error.set({});
}
