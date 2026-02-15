#ifndef CLEANGRADUATOR_ANGLEFROMVIDEOINTERACTOR_H
#define CLEANGRADUATOR_ANGLEFROMVIDEOINTERACTOR_H
#include <mutex>
#include <vector>

#include "../../domain/ports/angle/IAngleSource.h"
#include "../../domain/ports/video/IVideoSourceObserver.h"
#include "AngleFromVideoInteractorPorts.h"
#include "domain/fmt/Logger.h"

namespace domain::ports {
    struct IAngleCalculator;
}

namespace application::orchestrators {

    class AngleFromVideoInteractor final
        : domain::ports::IVideoSourceObserver
        , public domain::ports::IAngleSource
    {
    public:
        explicit AngleFromVideoInteractor(AngleFromVideoInteractorPorts ports);
        ~AngleFromVideoInteractor() override;

    public:
        // IAngleSource
        void start() override;
        void stop() override;
        void addSink(domain::ports::IAngleSink& sink) override;
        void removeSink(domain::ports::IAngleSink& sink) override;

    private:
        // IVideoSink
        void onVideoFrame(const domain::common::VideoFramePacket&) override;
        void onVideoSourceOpened() override;
        void onVideoSourceOpenFailed(const domain::common::VideoSourceError &) override;
        void onVideoSourceClosed(const domain::common::VideoSourceError &) override;

    private:
        domain::ports::IVideoSource& video_source_;
        domain::ports::IAngleCalculator& anglemeter_;
        fmt::Logger logger_;

        std::mutex mutex_;
        bool running_{false};
        std::vector<domain::ports::IAngleSink*> sinks_;
    };

} // namespace application

#endif //CLEANGRADUATOR_ANGLEFROMVIDEOINTERACTOR_H