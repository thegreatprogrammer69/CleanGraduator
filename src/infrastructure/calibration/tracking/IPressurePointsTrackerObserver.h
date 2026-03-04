#ifndef CLEANGRADUATOR_IPRESSUREPOINTSTRACKEROBSERVER_H
#define CLEANGRADUATOR_IPRESSUREPOINTSTRACKEROBSERVER_H

namespace infra::calib::tracking {
    struct PressurePointsTrackerEvent;

    struct IPressurePointsTrackerObserver {
        virtual ~IPressurePointsTrackerObserver() noexcept = default;
        virtual void onPressurePointsTrackerEvent(const PressurePointsTrackerEvent&) = 0;
    };
}

#endif //CLEANGRADUATOR_IPRESSUREPOINTSTRACKEROBSERVER_H