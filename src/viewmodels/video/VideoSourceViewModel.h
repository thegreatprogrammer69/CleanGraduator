#ifndef CLEANGRADUATOR_VIDEOSOURCEVIEWMODEL_H
#define CLEANGRADUATOR_VIDEOSOURCEVIEWMODEL_H
#include "application/dto/settings/grid/VideoSourceCrosshair.h"
#include "domain/events/IEventListener.h"
#include "domain/events/video/VideoSourceClosed.h"
#include "domain/ports/outbound/IVideoSink.h"
#include "viewmodels/Observable.h"
#include <string>

namespace domain::events {
    class IEventBus;
}

namespace domain::ports {
    struct IVideoSource;
}

namespace mvvm {
    class VideoSourceViewModel final : domain::ports::IVideoSink, domain::events::IEventListener
    {

    public:
        Observable<domain::common::VideoFramePtr> frame{};
        Observable<application::dto::VideoSourceCrosshair> crosshair{};
        Observable<bool> is_opened{false};
        Observable<std::string> error{};

    public:
        explicit VideoSourceViewModel(domain::ports::IVideoSource& video_source, domain::events::IEventBus& event_bus);
        ~VideoSourceViewModel() override;

    private:
        // IVideoSink
        void onVideoFrame(const domain::common::Timestamp &, domain::common::VideoFramePtr) override;

        // IEventListener
        void onEvent(const domain::events::Event *event) override;

    private:
        domain::ports::IVideoSource &video_source_;
        domain::events::IEventBus& event_bus_;
    };
}

#endif //CLEANGRADUATOR_VIDEOSOURCEVIEWMODEL_H
