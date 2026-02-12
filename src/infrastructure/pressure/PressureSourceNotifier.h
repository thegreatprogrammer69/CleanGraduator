#ifndef CLEANGRADUATOR_PRESSURESOURCENOTIFIER_H
#define CLEANGRADUATOR_PRESSURESOURCENOTIFIER_H

#include <algorithm>
#include <vector>

namespace domain::common {
    struct PressurePacket;
    struct PressureSourceError;
}

namespace domain::ports {
    struct IPressureSourceObserver;
}

namespace infra::pressure::detail {

    class PressureSourceNotifier {
    public:
        void addObserver(domain::ports::IPressureSourceObserver& observer);
        void removeObserver(domain::ports::IPressureSourceObserver& observer);

        void notifyPressure(const domain::common::PressurePacket& packet);
        void notifyOpened();
        void notifyOpenFailed(const domain::common::PressureSourceError& error);
        void notifyClosed(const domain::common::PressureSourceError& error);

    private:
        std::vector<domain::ports::IPressureSourceObserver*> observers_;
    };

}

#endif // CLEANGRADUATOR_PRESSURESOURCENOTIFIER_H
