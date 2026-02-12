#ifndef CLEANGRADUATOR_ANGLEFROMVIDEOINTERACTOR_H
#define CLEANGRADUATOR_ANGLEFROMVIDEOINTERACTOR_H
#include <mutex>
#include <vector>

#include "domain/ports/inbound/IAngleSource.h"
#include "domain/ports/outbound/IVideoSourceObserver.h"
#include "AngleFromVideoInteractorPorts.h"
#include "domain/fmt/FmtLogger.h"

namespace domain::ports {
    struct IAngleCalculator;
}

namespace application::interactors {

    class AngleFromVideoInteractor final
        : public domain::ports::IVideoSourceObserver
        , public domain::ports::IAngleSource
    {
    public:
        explicit AngleFromVideoInteractor(AngleFromVideoInteractorPorts ports);

        // IVideoSink
        void onVideoFrame(const domain::common::Timestamp &, domain::common::VideoFramePtr) override;

        // IAngleSource
        void start() override;
        void stop() override;
        void addSink(domain::ports::IAngleSink& sink) override;
        void removeSink(domain::ports::IAngleSink& sink) override;

    private:
        domain::ports::IAngleCalculator& anglemeter_;
        fmt::FmtLogger logger_;

        std::mutex mutex_;
        bool running_{false};
        std::vector<domain::ports::IAngleSink*> sinks_;
    };

} // namespace application

#endif //CLEANGRADUATOR_ANGLEFROMVIDEOINTERACTOR_H