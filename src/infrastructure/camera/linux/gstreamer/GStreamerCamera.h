#ifndef CLEANGRADUATOR_GSTREAMERCAMERA_H
#define CLEANGRADUATOR_GSTREAMERCAMERA_H

#include <atomic>
#include <mutex>
#include <thread>
#include <vector>
#include <string>

#include "domain/fmt/FmtLogger.h"
#include "domain/ports/inbound/IVideoSource.h"
#include "GStreamerCameraConfig.h"
#include "infrastructure/camera/CameraPorts.h"

typedef struct _GstElement GstElement;
typedef struct _GstBus     GstBus;
typedef struct _GstSample  GstSample;

namespace infra::camera {


    class GStreamerCamera final : public domain::ports::IVideoSource {
    public:
        explicit GStreamerCamera(const CameraPorts& ports, GStreamerCameraConfig config);
        ~GStreamerCamera() override;

        void open() override;
        void close() override;
        void addSink(domain::ports::IVideoSink& sink) override;
        void removeSink(domain::ports::IVideoSink& sink) override;

    private:
        void captureLoop();
        void dispatchSample(GstSample* sample);

    private:
        fmt::FmtLogger logger_;

        const CameraPorts& ports_;
        GStreamerCameraConfig config_;

        std::mutex sinks_mutex_;
        std::vector<domain::ports::IVideoSink*> sinks_;

        std::atomic<bool> running_{false};
        std::thread thread_;

        GstElement* pipeline_ = nullptr;
        GstElement* appsink_  = nullptr;
        GstBus*     bus_      = nullptr;
    };

} // namespace infra::camera

#endif // CLEANGRADUATOR_GSTREAMERCAMERA_H
