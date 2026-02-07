#ifndef CLEANGRADUATOR_V4LCAMERA_H
#define CLEANGRADUATOR_V4LCAMERA_H

#include <atomic>
#include <mutex>
#include <thread>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>

#include "domain/ports/inbound/IVideoSource.h"
#include "infrastructure/camera/CameraConfig.h"
#include "infrastructure/camera/CameraPorts.h"
#include "infrastructure/logging/FmtLogger.h"


namespace infra::camera {

class V4LCamera final : public domain::ports::IVideoSource {
public:
    explicit V4LCamera(const CameraPorts& ports, CameraConfig config);
    ~V4LCamera() override;

    bool start() override;
    void stop() override;
    void addSink(domain::ports::IVideoSink& sink) override;
    void removeSink(domain::ports::IVideoSink& sink) override;

private:
    void captureLoop();
    void dispatchFrame(const uint8_t* data, size_t size);

private:
    const CameraPorts& ports_;
    CameraConfig config_;

    std::mutex sinks_mutex_;
    std::vector<domain::ports::IVideoSink*> sinks_;

    std::atomic<bool> running_{false};
    std::thread thread_;

    struct Buffer {
        void*  start = nullptr;
        size_t length = 0;
    };

    int fd_ = -1;
    std::vector<Buffer> buffers_;

    logging::FmtLogger
};

}

#endif //CLEANGRADUATOR_V4LCAMERA_H