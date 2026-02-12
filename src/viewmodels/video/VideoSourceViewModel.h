#ifndef CLEANGRADUATOR_VIDEOSOURCEVIEWMODEL_H
#define CLEANGRADUATOR_VIDEOSOURCEVIEWMODEL_H
#include <string>

#include "domain/ports/outbound/IVideoSourceObserver.h"
#include "viewmodels/Observable.h"
#include "domain/core/video/VideoFrame.h"
#include "application/settings/video/CrosshairSettings.h"

namespace domain::ports {
    struct IVideoSource;
}

namespace mvvm {
    class VideoSourceViewModel final : domain::ports::IVideoSourceObserver {
    public:
        Observable<domain::common::VideoFramePtr> frame{};
        Observable<application::settings::CrosshairSettings> crosshair{};
        Observable<bool> is_opened{false};
        Observable<std::string> error{};

    public:
        explicit VideoSourceViewModel(domain::ports::IVideoSource& video_source);
        ~VideoSourceViewModel() override;

    private:
        // IVideoSourceObserver
        void onVideoFrame(const domain::common::VideoFramePacket &) override;
        void onVideoSourceOpened() override;
        void onVideoSourceOpenFailed(const domain::common::VideoSourceOpenError &) override;
        void onVideoSourceClosed() override;

    private:
        domain::ports::IVideoSource &video_source_;
    };
}

#endif //CLEANGRADUATOR_VIDEOSOURCEVIEWMODEL_H
