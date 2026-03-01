#ifndef CLEANGRADUATOR_VIDEOSOURCEVIEWMODEL_H
#define CLEANGRADUATOR_VIDEOSOURCEVIEWMODEL_H

#include <string>

#include "domain/core/video/VideoFramePacket.h"
#include "domain/core/video/VideoSourceEvent.h"
#include "domain/ports/video/IVideoSink.h"
#include "domain/ports/video/IVideoSourceObserver.h"
#include "viewmodels/Observable.h"

namespace domain::ports {
struct IVideoSource;
}

namespace mvvm {

class VideoSourceViewModel final : public domain::ports::IVideoSourceObserver, public domain::ports::IVideoSink {
public:
    Observable<domain::common::VideoFramePtr> frame{};
    Observable<bool> is_opened{false};
    Observable<std::string> error{};

    explicit VideoSourceViewModel(domain::ports::IVideoSource& video_source);
    ~VideoSourceViewModel() override;

private:
    void onVideoSourceEvent(const domain::common::VideoSourceEvent& event) override;
    void onVideoFrame(const domain::common::VideoFramePacket& frame_packet) override;

private:
    domain::ports::IVideoSource& video_source_;
};

} // namespace mvvm

#endif //CLEANGRADUATOR_VIDEOSOURCEVIEWMODEL_H
