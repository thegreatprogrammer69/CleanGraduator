#ifndef CLEANGRADUATOR_GSTREAMERCAMERA_H
#define CLEANGRADUATOR_GSTREAMERCAMERA_H

#include <atomic>
#include <mutex>
#include <thread>
#include <vector>
#include <string>

#include "domain/fmt/Logger.h"
#include "domain/ports/inbound/IVideoSource.h"
#include "GStreamerCameraConfig.h"
#include "infrastructure/video/VideoSourcePorts.h"
#include "infrastructure/video/VideoSourceNotifier.h"

typedef struct _GstElement GstElement;
typedef struct _GstBus     GstBus;
typedef struct _GstSample  GstSample;

namespace infra::camera {

    class GStreamerCamera final : public domain::ports::IVideoSource {
    public:
        explicit GStreamerCamera(const VideoSourcePorts& ports, GStreamerCameraConfig config);
        ~GStreamerCamera() override;

        bool open() override;
        void close() override;
        void addObserver(domain::ports::IVideoSourceObserver&) override;
        void removeObserver(domain::ports::IVideoSourceObserver&) override;

    private:
        void captureLoop();
        void dispatchSample(GstSample* sample);

    private:
        detail::VideoSourceNotifier notifier_;

        fmt::Logger logger_;

        const VideoSourcePorts& ports_;
        GStreamerCameraConfig config_;

        std::atomic<bool> running_{false};
        std::thread thread_;

        GstElement* pipeline_ = nullptr;
        GstElement* appsink_  = nullptr;
        GstBus*     bus_      = nullptr;
    };

} // namespace infra::camera

#endif // CLEANGRADUATOR_GSTREAMERCAMERA_H
