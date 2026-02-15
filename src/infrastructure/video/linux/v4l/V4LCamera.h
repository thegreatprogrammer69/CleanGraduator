#ifndef CLEANGRADUATOR_V4LCAMERA_H
#define CLEANGRADUATOR_V4LCAMERA_H

#include <atomic>
#include <mutex>
#include <thread>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>

#include "domain/fmt/Logger.h"
#include "../../../../domain/ports/video/IVideoSource.h"
#include "V4LCameraConfig.h"
#include "infrastructure/video/VideoSourceNotifier.h"
#include "infrastructure/video/VideoSourcePorts.h"


namespace domain::ports {
    struct IVideoSourceObserver;
}

namespace infra::camera {

class V4LCamera final : public domain::ports::IVideoSource {
public:
    explicit V4LCamera(VideoSourcePorts ports, V4LCameraConfig config);
    ~V4LCamera() override;

    bool open() override;
    void close() override;
    void addObserver(domain::ports::IVideoSourceObserver&) override;
    void removeObserver(domain::ports::IVideoSourceObserver&) override;

private:
    void captureLoop();
    void dispatchFrame(const uint8_t* data, size_t size);

private:
    detail::VideoSourceNotifier notifier_;

    fmt::Logger logger_;

    VideoSourcePorts ports_;
    V4LCameraConfig config_;

    std::atomic<bool> running_{false};
    std::thread thread_;

    struct Buffer {
        void*  start = nullptr;
        size_t length = 0;
    };

    int fd_ = -1;
    std::vector<Buffer> buffers_;

};

}

#endif //CLEANGRADUATOR_V4LCAMERA_H