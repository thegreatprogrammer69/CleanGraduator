#ifndef CLEANGRADUATOR_DSHOWVIDEOSTREAM_H
#define CLEANGRADUATOR_DSHOWVIDEOSTREAM_H

#include "domain/ports/video/IVideoSource.h"
#include "domain/fmt/Logger.h"
#include "DShowCameraConfig.h"
#include "infrastructure/video/VideoSourceNotifier.h"
#include "infrastructure/video/VideoSourcePorts.h"
#include "infrastructure/utils/thread/ThreadWorker.h"

#include <memory>
#include <mutex>
#include <atomic>
#include <string>

namespace infra::camera {

    class DShowCamera final : public domain::ports::IVideoSource {
    public:
        explicit DShowCamera(VideoSourcePorts ports, DShowCameraConfig config);
        ~DShowCamera() override;

        void open() override;
        void close() override;
        bool isRunning() const noexcept override;

        void addObserver(domain::ports::IVideoSourceObserver&) override;
        void removeObserver(domain::ports::IVideoSourceObserver&) override;

        void addSink(domain::ports::IVideoSink&) override;
        void removeSink(domain::ports::IVideoSink&) override;

        // вызывается из SampleGrabberCB
        void onFrame(double time, unsigned char* data, long size);

    private:
        struct DShowCameraImpl;

        bool initializeCom();
        bool createGraph();
        bool createSourceFilter();
        bool createSampleGrabber();
        bool createNullRenderer();
        bool connectGraph();
        bool readConnectedFormat();
        bool configureSampleGrabber();
        bool runGraph();

        void startWatchdog();
        void stopWatchdog();
        void watchdogTick();

        void failOpen(const std::string& reason);
        void failRuntime(const std::string& reason);

    private:
        mutable std::mutex state_mutex_;
        std::unique_ptr<DShowCameraImpl> impl_;

        fmt::Logger logger_;
        domain::ports::IClock& clock_;
        detail::VideoSourceNotifier notifier_;
        DShowCameraConfig config_;

        utils::thread::ThreadWorker watchdog_worker_;
        std::atomic<bool> watchdog_enabled_{false};
    };

} // namespace infra::camera

#endif // CLEANGRADUATOR_DSHOWVIDEOSTREAM_H