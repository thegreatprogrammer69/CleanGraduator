#ifndef CLEANGRADUATOR_IANGLESINK_H
#define CLEANGRADUATOR_IANGLESINK_H

namespace domain::common {
    struct AngleSourceEvent;
    struct AngleSourcePacket;
}

namespace domain::ports {

    struct IAngleSourceObserver {
        virtual ~IAngleSourceObserver() = default;
        virtual void onAngleSourceEvent(const common::AngleSourceEvent&) = 0;
    };

}

#endif //CLEANGRADUATOR_IANGLESINK_H