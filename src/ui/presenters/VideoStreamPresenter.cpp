#include "VideoStreamPresenter.h"

#include <QMetaObject>
#include <QThread>

VideoStreamPresenter::VideoStreamPresenter() = default;

VideoStreamPresenter::~VideoStreamPresenter() = default;

IVideoSink& VideoStreamPresenter::sink() {
    return *this;
}

VideoFramePtr VideoStreamPresenter::lastVideoFrame() {
    std::lock_guard lock(mutex_);
    return latest_frame_;
}

void VideoStreamPresenter::onVideoFrame(time_point_t, VideoFramePtr frame) {
    {
        std::lock_guard lock(mutex_);
        latest_frame_ = std::move(frame);
    }

    // ВАЖНО:
    // onVideoFrame может приходить НЕ из GUI thread.
    // Сигнал надо эмитить безопасно для Qt.
    if (QThread::currentThread() == thread()) {
        emit videoFrameReady();
    } else {
        QMetaObject::invokeMethod(
            this,
            [this]() { emit videoFrameReady(); },
            Qt::QueuedConnection
        );
    }
}
