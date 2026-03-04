#ifndef CLEANGRADUATOR_IANGLESINK_H
#define CLEANGRADUATOR_IANGLESINK_H

namespace domain::common {
    struct AngleSourcePacket;
}

namespace domain::ports {
    struct IAngleSink {
        virtual ~IAngleSink() = default;
        virtual void onAnglePacket(const common::AngleSourcePacket&) = 0;
    };
}

#endif //CLEANGRADUATOR_IANGLESINK_H