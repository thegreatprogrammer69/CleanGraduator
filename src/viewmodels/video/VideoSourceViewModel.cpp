#include "VideoSourceViewModel.h"

#include "domain/core/video/VideoFramePacket.h"
#include "domain/core/video/VideoSourceOpenError.h"
#include "domain/ports/inbound/IVideoSource.h"

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

void VideoSourceViewModel::onVideoSourceOpenFailed(const domain::common::VideoSourceOpenError & err) {
    is_opened.set(false);
    error.set(err.message);
}

void VideoSourceViewModel::onVideoSourceClosed() {
    is_opened.set(false);
}

void VideoSourceViewModel::onVideoFrame(const domain::common::VideoFramePacket& frame_packet) {
    frame.set(frame_packet.frame);
}
