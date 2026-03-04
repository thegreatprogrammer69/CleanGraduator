#ifndef CLEANGRADUATOR_FAKEANGLESOURCEFROMVIDEO_H
#define CLEANGRADUATOR_FAKEANGLESOURCEFROMVIDEO_H

#include <atomic>
#include <cstdint>

#include "domain/core/angle/AngleSourceId.h"
#include "domain/fmt/Logger.h"
#include "domain/ports/angle/IAngleSource.h"
#include "domain/ports/video/IVideoSink.h"
#include "infrastructure/angle/AngleSourcePorts.h"
#include "shared/ThreadSafeObserverList.h"

namespace domain::ports {
    struct IVideoSource;
}

namespace infra::angle {

    struct FakeAngleSourceFromVideoConfig {
        double from_deg{0.0};
        double to_deg{360.0};
        std::uint64_t duration_ms{10000};
    };

    class FakeAngleSourceFromVideo final
        : public domain::ports::IVideoSink
        , public domain::ports::IAngleSource
    {
    public:
        FakeAngleSourceFromVideo(
            domain::common::AngleSourceId id,
            AngleSourcePorts ports,
            FakeAngleSourceFromVideoConfig config);

        ~FakeAngleSourceFromVideo() override;

        // IAngleSource
        void start() override;
        void stop() override;
        bool isRunning() const noexcept override;

        void addSink(domain::ports::IAngleSink&) override;
        void removeSink(domain::ports::IAngleSink&) override;

        void addObserver(domain::ports::IAngleSourceObserver&) override;
        void removeObserver(domain::ports::IAngleSourceObserver&) override;

        // IVideoSink
        void onVideoFrame(const domain::common::VideoFramePacket&) override;

    private:
        enum class State : std::uint8_t { Stopped, Started };

        double computeAngle(std::uint64_t timestamp_ms) const;

        domain::common::AngleSourceId id_;
        domain::ports::IVideoSource& video_source_;
        fmt::Logger logger_;
        FakeAngleSourceFromVideoConfig config_;

        std::atomic<State> state_{State::Stopped};

        ThreadSafeObserverList<domain::ports::IAngleSink> sinks_;
        ThreadSafeObserverList<domain::ports::IAngleSourceObserver> observers_;
    };

}

#endif