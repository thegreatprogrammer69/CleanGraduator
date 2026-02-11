#include "VideoSourceViewModel.h"

#include <iostream>
#include <ostream>

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
    std::cout << "VideoSourceViewModel::onVideoFrame()" << std::endl;
    frame.set(_frame);
}

void mvvm::VideoSourceViewModel::onCrosshairChanged(const application::dto::VideoSourceCrosshair& _crosshair) {
    crosshair.set(_crosshair);
}

void mvvm::VideoSourceViewModel::onSourceOpened() {
    is_opened.set(true);
}

void mvvm::VideoSourceViewModel::onSourceClosed() {
    is_opened.set(false);
}
