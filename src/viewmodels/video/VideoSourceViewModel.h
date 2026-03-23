#ifndef CLEANGRADUATOR_VIDEOSOURCEVIEWMODEL_H
#define CLEANGRADUATOR_VIDEOSOURCEVIEWMODEL_H
#include <string>

#include "../../domain/ports/video/IVideoSourceObserver.h"
#include "viewmodels/Observable.h"
#include "domain/core/video/VideoFrame.h"
#include "domain/core/video/VideoFramePacket.h"
#include "domain/ports/video/IVideoSink.h"
#include "viewmodels/video/ICircleOverlayConfigurable.h"

namespace domain::ports {
    struct IVideoSource;
}

namespace mvvm {
    class VideoSourceViewModel final : public domain::ports::IVideoSourceObserver, public domain::ports::IVideoSink, public ICircleOverlayConfigurable {
    public:
        Observable<domain::common::VideoFramePtr> frame{};
        Observable<bool> is_opened{false};
        Observable<std::string> error{};
        Observable<CircleOverlaySettings> circle_overlay_settings{CircleOverlaySettings{}};

    public:
        explicit VideoSourceViewModel(domain::ports::IVideoSource& video_source);
        ~VideoSourceViewModel() override;

    public:
        Observable<CircleOverlaySettings>& circleOverlaySettings() override { return circle_overlay_settings; }
        const Observable<CircleOverlaySettings>& circleOverlaySettings() const override { return circle_overlay_settings; }

    private:
        // IVideoSourceObserver
        void onVideoFrame(const domain::common::VideoFramePacket &) override;
        void onVideoSourceEvent(const domain::common::VideoSourceEvent &) override;

    private:
        domain::ports::IVideoSource &video_source_;
    };
}

#endif //CLEANGRADUATOR_VIDEOSOURCEVIEWMODEL_H
