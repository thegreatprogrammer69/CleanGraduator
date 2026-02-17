#ifndef CLEANGRADUATOR_IANGLESINK_H
#define CLEANGRADUATOR_IANGLESINK_H

namespace domain::common {
    struct AnglePacket;
}

namespace domain::ports {
    struct IAngleSink {
        virtual ~IAngleSink() = default;
        virtual void onAngle(const common::AnglePacket&) noexcept = 0;
    };
}

#endif //CLEANGRADUATOR_IANGLESINK_H