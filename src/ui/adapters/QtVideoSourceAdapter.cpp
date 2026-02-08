#include "QtVideoSourceAdapter.h"

#include <QMetaObject>
#include <QThread>

#include "domain/ports/inbound/IVideoSource.h"

using namespace ui::adapters;

QtVideoSourceAdapter::QtVideoSourceAdapter(domain::ports::IVideoSource& video_source)
    : video_source_(video_source)
{
    video_source_.addSink(*this);
}

QtVideoSourceAdapter::~QtVideoSourceAdapter() {
    video_source_.removeSink(*this);
}


domain::common::VideoFramePtr QtVideoSourceAdapter::lastVideoFrame() const {
    std::lock_guard lock(mutex_);
    return latest_frame_;
}

void QtVideoSourceAdapter::onVideoFrame(const domain::common::Timestamp&, domain::common::VideoFramePtr frame) {
    {
        std::lock_guard lock(mutex_);
        latest_frame_ = std::move(frame);
    }
    QMetaObject::invokeMethod(
        this,
        [this] { emit videoFrameReady(); },
        Qt::QueuedConnection
    );
}