#ifndef CLEANGRADUATOR_DSHOWVIDEOSTREAM_H
#define CLEANGRADUATOR_DSHOWVIDEOSTREAM_H

#include "domain/ports/inbound/IVideoSource.h"
#include "domain/fmt/FmtLogger.h"
#include "DShowCameraConfig.h"
#include "infrastructure/video/VideoSourceNotifier.h"
#include "infrastructure/video/VideoSourcePorts.h"

namespace infra::camera {

class DShowCamera final : public domain::ports::IVideoSource {
public:
    explicit DShowCamera(VideoSourcePorts ports, DShowCameraConfig config);
    ~DShowCamera() override;
    void open() override;
    void close() override;
    void addObserver(domain::ports::IVideoSourceObserver &) override;
    void removeObserver(domain::ports::IVideoSourceObserver &) override;

    // вызывается из SampleGrabberCB
    void onFrame(double time, unsigned char* data, long size);


private:
    struct DShowCameraImpl;
    std::unique_ptr<DShowCameraImpl> impl_;

    fmt::FmtLogger logger_;
    domain::ports::IClock& clock_;
    detail::VideoSourceNotifier notifier_;

    DShowCameraConfig config_;
};

}

#endif //CLEANGRADUATOR_DSHOWVIDEOSTREAM_H