#ifndef CLEANGRADUATOR_DSHOWVIDEOSTREAM_H
#define CLEANGRADUATOR_DSHOWVIDEOSTREAM_H
#include <mutex>
#include <vector>
#include "domain/ports/inbound/IVideoSource.h"
#include "domain/ports/outbound/IClock.h"
#include "domain/ports/outbound/ILogger.h"
#include "infrastructure/camera/CameraConfig.h"
#include "infrastructure/camera/CameraPorts.h"

namespace infra::camera {

class DShowCameraStream final : public IVideoSource {
public:
    explicit DShowCameraStream(const CameraPorts& ports, const CameraConfig& config);
    ~DShowCameraStream() override;
    bool start() override;
    void stop() override;
    void addSink(IVideoSink& sink) override;
    void removeSink(IVideoSink& sink) override;

    // вызывается из SampleGrabberCB
    void onFrame(double time, unsigned char* data, long size);

private:
    struct DShowCameraStreamImpl;
    std::unique_ptr<DShowCameraStreamImpl> impl_;
    CameraConfig config_;
    const CameraPorts& ports_;
    std::mutex sinks_mutex_;
    std::vector<IVideoSink*> sinks_;
};

}

#endif //CLEANGRADUATOR_DSHOWVIDEOSTREAM_H