#include "VideoSourceViewModel.h"
#include "domain/ports/inbound/IVideoSource.h"

mvvm::VideoSourceViewModel::VideoSourceViewModel(domain::ports::IVideoSource &video_source)
    : video_source_(video_source)
{
    video_source_.addSink(*this);
}

mvvm::VideoSourceViewModel::~VideoSourceViewModel() {
    video_source_.removeSink(*this);
}

void mvvm::VideoSourceViewModel::onVideoFrame(const domain::common::Timestamp &, domain::common::VideoFramePtr _frame) {
    frame.set(_frame);
}
