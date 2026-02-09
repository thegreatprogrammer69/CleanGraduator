#ifndef CLEANGRADUATOR_VIDEOSOURCEVIEWMODEL_H
#define CLEANGRADUATOR_VIDEOSOURCEVIEWMODEL_H
#include "Observable.h"
#include "domain/ports/outbound/IVideoSink.h"

namespace domain::ports {
    struct IVideoSource;
}

namespace mvvm {
    class VideoSourceViewModel final : domain::ports::IVideoSink {
    public:
        Observable<domain::common::VideoFramePtr> frame;

    public:
        explicit VideoSourceViewModel(domain::ports::IVideoSource& video_source);
        ~VideoSourceViewModel() override;

    private:
        // IVideoSink
        void onVideoFrame(const domain::common::Timestamp &, domain::common::VideoFramePtr) override;

    private:
        domain::ports::IVideoSource &video_source_;
    };
}

#endif //CLEANGRADUATOR_VIDEOSOURCEVIEWMODEL_H
