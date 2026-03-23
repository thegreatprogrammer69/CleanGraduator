#ifndef CLEANGRADUATOR_VIDEOSOURCEVIEWMODEL_H
#define CLEANGRADUATOR_VIDEOSOURCEVIEWMODEL_H
#include <string>

#include "../../domain/ports/video/IVideoSourceObserver.h"
#include "viewmodels/Observable.h"
#include "viewmodels/video/IVideoSourceWidgetViewModel.h"
#include "domain/core/video/VideoFrame.h"
#include "domain/core/video/VideoFramePacket.h"
#include "domain/ports/video/IVideoSink.h"

namespace domain::ports {
    struct IVideoSource;
}

namespace mvvm {
    class VideoSourceViewModel final : public IVideoSourceWidgetViewModel, public domain::ports::IVideoSourceObserver, public domain::ports::IVideoSink {
    public:
        Observable<domain::common::VideoFramePtr> frame{};
        Observable<bool> is_opened{false};
        Observable<std::string> error{};
        Observable<int> circle_diameter_percent{15};
        Observable<std::uint32_t> circle_color1{0xFFFFFFFFu};
        Observable<std::uint32_t> circle_color2{0x000000FFu};

    public:
        explicit VideoSourceViewModel(domain::ports::IVideoSource& video_source);
        ~VideoSourceViewModel() override;


        Observable<domain::common::VideoFramePtr>& frameStream() override { return frame; }
        const Observable<domain::common::VideoFramePtr>& frameStream() const override { return frame; }

        Observable<bool>& openedState() override { return is_opened; }
        const Observable<bool>& openedState() const override { return is_opened; }

        Observable<std::string>& errorState() override { return error; }
        const Observable<std::string>& errorState() const override { return error; }

        Observable<int>& circleDiameterPercent() override { return circle_diameter_percent; }
        const Observable<int>& circleDiameterPercent() const override { return circle_diameter_percent; }

        Observable<std::uint32_t>& circleColor1() override { return circle_color1; }
        const Observable<std::uint32_t>& circleColor1() const override { return circle_color1; }

        Observable<std::uint32_t>& circleColor2() override { return circle_color2; }
        const Observable<std::uint32_t>& circleColor2() const override { return circle_color2; }

    private:
        // IVideoSourceObserver
        void onVideoFrame(const domain::common::VideoFramePacket &) override;
        void onVideoSourceEvent(const domain::common::VideoSourceEvent &) override;

    private:
        domain::ports::IVideoSource &video_source_;
    };
}

#endif //CLEANGRADUATOR_VIDEOSOURCEVIEWMODEL_H
