#ifndef CLEANGRADUATOR_VIDEOSOURCEVIEWMODEL_H
#define CLEANGRADUATOR_VIDEOSOURCEVIEWMODEL_H
#include "../Observable.h"
#include "application/dto/settings/camera_grid/VideoSourceCrosshair.h"
#include "application/fmt/fmt_application.h"
#include "application/ports/outbound/video/IVideoSourceCrosshairListener.h"
#include "application/ports/outbound/video/IVideoSourceLifecycleObserver.h"
#include "domain/ports/outbound/IVideoSink.h"

namespace domain::ports {
    struct IVideoSource;
}

namespace mvvm {
    class VideoSourceViewModel final
        : domain::ports::IVideoSink
        , protected application::ports::IVideoSourceCrosshairListener
        , protected application::ports::IVideoSourceLifecycleObserver
    {

    public:
        Observable<domain::common::VideoFramePtr> frame;
        Observable<application::dto::VideoSourceCrosshair> crosshair;
        Observable<bool> is_opened;

    public:
        explicit VideoSourceViewModel(domain::ports::IVideoSource& video_source);
        ~VideoSourceViewModel() override;

    protected:
        // IVideoSourceCrosshairPort
        void onCrosshairChanged(const application::dto::VideoSourceCrosshair&) override;

        void onSourceOpened() override;
        void onSourceClosed() override;

    private:
        // IVideoSink
        void onVideoFrame(const domain::common::Timestamp &, domain::common::VideoFramePtr) override;

    private:
        domain::ports::IVideoSource &video_source_;
    };
}

#endif //CLEANGRADUATOR_VIDEOSOURCEVIEWMODEL_H
