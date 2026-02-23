#ifndef CLEANGRADUATOR_IPRESSUREPOINTSTRACKEROBSERVER_H
#define CLEANGRADUATOR_IPRESSUREPOINTSTRACKEROBSERVER_H

namespace domain::common {
    struct IPressurePointsTrackerObserver {
        virtual ~IPressurePointsTrackerObserver() = default;

        virtual void onPointEntered(int index) = 0;
        virtual void onPointExited(int index) = 0;
    };
}

#endif //CLEANGRADUATOR_IPRESSUREPOINTSTRACKEROBSERVER_H