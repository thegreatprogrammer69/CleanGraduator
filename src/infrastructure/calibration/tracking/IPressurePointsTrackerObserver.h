#ifndef CLEANGRADUATOR_IPRESSUREPOINTSTRACKEROBSERVER_H
#define CLEANGRADUATOR_IPRESSUREPOINTSTRACKEROBSERVER_H

namespace domain::common {
    struct PressurePointsTrackerEvent;
}

namespace domain::ports {

    struct IPressurePointsTrackerObserver {
        virtual ~IPressurePointsTrackerObserver() noexcept = default;

        virtual void onPressurePointsTrackerEvent(const common::PressurePointsTrackerEvent&) = 0;
    };

}

#endif //CLEANGRADUATOR_IPRESSUREPOINTSTRACKEROBSERVER_H