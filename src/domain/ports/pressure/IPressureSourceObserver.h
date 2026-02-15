#ifndef CLEANGRADUATOR_IPRESSURESOURCEOBSERVER_H
#define CLEANGRADUATOR_IPRESSURESOURCEOBSERVER_H

namespace domain::common {
    struct PressureSourceError;
    struct PressurePacket;
}

namespace domain::ports {
    struct IPressureSourceObserver {
        virtual ~IPressureSourceObserver() = default;
        virtual void onPressurePacket(const common::PressurePacket&) = 0;
        virtual void onPressureSourceOpened() = 0;
        virtual void onPressureSourceOpenFailed(const common::PressureSourceError&) = 0;
        virtual void onPressureSourceClosed(const common::PressureSourceError&) = 0;
    };
}

#endif //CLEANGRADUATOR_IPRESSURESOURCEOBSERVER_H