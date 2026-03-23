#ifndef CLEANGRADUATOR_VIDEOSOURCEVIEWMODEL_H
#define CLEANGRADUATOR_VIDEOSOURCEVIEWMODEL_H
#include <string>

#include "../../domain/ports/video/IVideoSourceObserver.h"
#include "viewmodels/Observable.h"
#include "viewmodels/video/ICircleOverlaySettingsViewModel.h"
#include "domain/core/video/VideoFrame.h"
#include "domain/core/video/VideoFramePacket.h"
#include "domain/ports/video/IVideoSink.h"

namespace domain::ports {
    struct IVideoSource;
}

namespace mvvm {
    class VideoSourceViewModel final
        : public domain::ports::IVideoSourceObserver
        , public domain::ports::IVideoSink
        , public ICircleOverlaySettingsViewModel {
    public:
        Observable<domain::common::VideoFramePtr> frame{};
        Observable<bool> is_opened{false};
        Observable<std::string> error{};

    public:
        explicit VideoSourceViewModel(
            domain::ports::IVideoSource& video_source,
            ICircleOverlaySettingsViewModel& circle_overlay_settings);
        ~VideoSourceViewModel() override;

        Observable<int>& circleDiameterPercent() override;
        const Observable<int>& circleDiameterPercent() const override;

        Observable<std::uint32_t>& circleColor1() override;
        const Observable<std::uint32_t>& circleColor1() const override;

        Observable<std::uint32_t>& circleColor2() override;
        const Observable<std::uint32_t>& circleColor2() const override;

    private:
        // IVideoSourceObserver
        void onVideoFrame(const domain::common::VideoFramePacket &) override;
        void onVideoSourceEvent(const domain::common::VideoSourceEvent &) override;

    private:
        domain::ports::IVideoSource &video_source_;
        ICircleOverlaySettingsViewModel& circle_overlay_settings_;
    };
}

#endif //CLEANGRADUATOR_VIDEOSOURCEVIEWMODEL_H
