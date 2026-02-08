#ifndef CLEANGRADUATOR_VIDEOSTREAMPRESENTER_H
#define CLEANGRADUATOR_VIDEOSTREAMPRESENTER_H
#include <mutex>
#include <QObject>
#include "domain/ports/outbound/IVideoSink.h"

namespace ui::presenters {
class VideoStreamPresenter final : public QObject, domain::ports::IVideoSink {
    Q_OBJECT
public:
    VideoStreamPresenter();
    ~VideoStreamPresenter() override;

    domain::common::VideoFramePtr lastVideoFrame();
    IVideoSink& sink();

private: // IVideoSink
    void onVideoFrame(const domain::common::Timestamp&, domain::common::VideoFramePtr) override;

signals:
    void videoFrameReady();

private:
    std::mutex mutex_;
    domain::common::VideoFramePtr latest_frame_;
};

}

#endif //CLEANGRADUATOR_VIDEOSTREAMPRESENTER_H