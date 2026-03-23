#ifndef CLEANGRADUATOR_VIDEOSOURCEVIEWMODEL_H
#define CLEANGRADUATOR_VIDEOSOURCEVIEWMODEL_H
#include <string>
#include <cstdint>

#include "../../domain/ports/video/IVideoSourceObserver.h"
#include "viewmodels/Observable.h"
#include "domain/core/video/VideoFrame.h"
#include "domain/core/video/VideoFramePacket.h"
#include "domain/ports/video/IVideoSink.h"
#include "viewmodels/video/ICircleOverlayViewModel.h"

namespace domain::ports {
    struct IVideoSource;
}

namespace mvvm {
    class VideoSourceViewModel final
        : public domain::ports::IVideoSourceObserver
        , public domain::ports::IVideoSink
        , public ICircleOverlayViewModel {
    public:
        Observable<domain::common::VideoFramePtr> frame{};
        Observable<bool> is_opened{false};
        Observable<std::string> error{};

        Observable<float> circle_diameter_percent{0.0f};
        Observable<std::uint32_t> circle_color1{0xFFFFFFFFu};
        Observable<std::uint32_t> circle_color2{0x000000FFu};

    public:
        explicit VideoSourceViewModel(domain::ports::IVideoSource& video_source);
        ~VideoSourceViewModel() override;

        Observable<float>& circleDiameterPercent() override;
        Observable<std::uint32_t>& circleColor1() override;
        Observable<std::uint32_t>& circleColor2() override;

    private:
        void onVideoFrame(const domain::common::VideoFramePacket &) override;
        void onVideoSourceEvent(const domain::common::VideoSourceEvent &) override;

    private:
        domain::ports::IVideoSource &video_source_;
    };
}

#endif //CLEANGRADUATOR_VIDEOSOURCEVIEWMODEL_H
