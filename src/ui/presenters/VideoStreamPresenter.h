#ifndef CLEANGRADUATOR_VIDEOSTREAMPRESENTER_H
#define CLEANGRADUATOR_VIDEOSTREAMPRESENTER_H
#include <mutex>
#include <QObject>
#include "domain/ports/outbound/IVideoSink.h"

class VideoStreamPresenter final : public QObject, private IVideoSink {
    Q_OBJECT
public:
    VideoStreamPresenter();
    ~VideoStreamPresenter() override;
    VideoFramePtr lastVideoFrame();
    IVideoSink& sink();

private: // IVideoSink
    void onVideoFrame(time_point_t, VideoFramePtr) override;

signals:
    void videoFrameReady();

private:
    std::mutex mutex_;
    VideoFramePtr latest_frame_;
};


#endif //CLEANGRADUATOR_VIDEOSTREAMPRESENTER_H