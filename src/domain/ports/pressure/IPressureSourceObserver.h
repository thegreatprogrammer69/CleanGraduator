#ifndef CLEANGRADUATOR_IPRESSURESOURCEOBSERVER_H
#define CLEANGRADUATOR_IPRESSURESOURCEOBSERVER_H

namespace domain::common {
    struct PressureSourceEvent;
}

namespace domain::ports {
    struct IPressureSourceObserver {
        virtual ~IPressureSourceObserver() noexcept = default;
        virtual void onPressureSourceEvent(const common::PressureSourceEvent&) = 0;
    };
}

#endif //CLEANGRADUATOR_IPRESSURESOURCEOBSERVER_H