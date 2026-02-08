#ifndef CLEANGRADUATOR_DSHOWVIDEOSTREAM_H
#define CLEANGRADUATOR_DSHOWVIDEOSTREAM_H
#include <mutex>
#include <vector>
#include "domain/ports/inbound/IVideoSource.h"
#include "domain/ports/outbound/IClock.h"
#include "domain/ports/outbound/ILogger.h"
#include "DShowCameraConfig.h"
#include "domain/fmt/FmtLogger.h"
#include "infrastructure/camera/CameraPorts.h"

namespace infra::camera {

class DShowCamera final : public domain::ports::IVideoSource {
public:
    explicit DShowCamera(CameraPorts ports, DShowCameraConfig config);
    ~DShowCamera() override;
    bool start() override;
    void stop() override;
    void addSink(domain::ports::IVideoSink& sink) override;
    void removeSink(domain::ports::IVideoSink& sink) override;

    // вызывается из SampleGrabberCB
    void onFrame(double time, unsigned char* data, long size);

private:
    struct DShowCameraImpl;
    std::unique_ptr<DShowCameraImpl> impl_;

    fmt::FmtLogger logger_;
    domain::ports::IClock& clock_;

    DShowCameraConfig config_;

    std::mutex sinks_mutex_;
    std::vector<domain::ports::IVideoSink*> sinks_;
};

}

#endif //CLEANGRADUATOR_DSHOWVIDEOSTREAM_H