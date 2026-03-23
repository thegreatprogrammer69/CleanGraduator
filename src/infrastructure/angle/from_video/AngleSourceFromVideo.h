#ifndef CLEANGRADUATOR_ANGLESOURCEFROMVIDEO_H
#define CLEANGRADUATOR_ANGLESOURCEFROMVIDEO_H
#include <atomic>
#include <cstdint>

#include "AngleSourceFromVideoConfig.h"
#include "domain/core/angle/SourceId.h"
#include "domain/fmt/Logger.h"
#include "domain/ports/angle/IAngleSource.h"
#include "domain/ports/video/IVideoSink.h"
#include "infrastructure/angle/AngleSourcePorts.h"
#include "../../../shared/list/ThreadSafeObserverList.h"

namespace domain::ports {
    struct IVideoSource;
    struct IAngleCalculator;
}

namespace infra::angle {
    class AngleSourceFromVideo final
        : public domain::ports::IVideoSink
        , public domain::ports::IAngleSource
    {
    public:
        explicit AngleSourceFromVideo(domain::common::SourceId id, AngleSourcePorts ports, AngleSourceFromVideoConfig config);
        ~AngleSourceFromVideo() override;

        // IAngleSource
        void start() override;
        void stop() override;
        bool isRunning() const noexcept override;

        void addSink(domain::ports::IAngleSink &) override;
        void removeSink(domain::ports::IAngleSink &) override;

        void addObserver(domain::ports::IAngleSourceObserver &) override;
        void removeObserver(domain::ports::IAngleSourceObserver &) override;

        void onVideoFrame(const domain::common::VideoFramePacket &) override;

    private:
        enum class State : std::uint8_t { Stopped, Started };

        domain::common::SourceId id_;

        domain::ports::IVideoSource&     video_source_;
        domain::ports::IAngleCalculator& anglemeter_;
        fmt::Logger                      logger_;

        std::atomic<State> state_{State::Stopped};

        ThreadSafeObserverList<domain::ports::IAngleSink> sinks_;
        ThreadSafeObserverList<domain::ports::IAngleSourceObserver> observers_;
    };
}

#endif //CLEANGRADUATOR_ANGLESOURCEFROMVIDEO_H