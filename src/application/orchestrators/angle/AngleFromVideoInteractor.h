#ifndef CLEANGRADUATOR_ANGLEFROMVIDEOINTERACTOR_H
#define CLEANGRADUATOR_ANGLEFROMVIDEOINTERACTOR_H

#include <atomic>
#include <mutex>
#include <vector>

#include "AngleFromVideoInteractorPorts.h"

#include "domain/ports/angle/IAngleSource.h"
#include "domain/ports/angle/IAngleSourceObserver.h"
#include "domain/ports/video/IVideoSourceObserver.h"

#include "domain/core/angle/AngleSourceId.h"
#include "domain/core/angle/AngleSourceError.h"
#include "domain/fmt/Logger.h"
#include "domain/ports/video/IVideoSink.h"
#include "shared/ThreadSafeObserverList.h"

namespace domain::ports {
    struct IVideoSource;
    struct IAngleCalculator;
}

namespace application::orchestrators {

class AngleFromVideoInteractor final
    : public domain::ports::IVideoSink
    , public domain::ports::IAngleSource
{
public:
    explicit AngleFromVideoInteractor(domain::common::AngleSourceId id,
                                      AngleFromVideoInteractorPorts ports);
    ~AngleFromVideoInteractor() override;

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
    enum class State : uint8_t { Stopped, Started };

    domain::common::AngleSourceId id_;

    domain::ports::IVideoSource&     video_source_;
    domain::ports::IAngleCalculator& anglemeter_;
    fmt::Logger                      logger_;

    std::atomic<State> state_{State::Stopped};

    ThreadSafeObserverList<domain::ports::IAngleSink> sinks_;
    ThreadSafeObserverList<domain::ports::IAngleSourceObserver> observers_;
};

} // namespace application::orchestrators

#endif // CLEANGRADUATOR_ANGLEFROMVIDEOINTERACTOR_H