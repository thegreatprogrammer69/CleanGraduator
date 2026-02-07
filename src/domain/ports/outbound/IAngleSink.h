#ifndef CLEANGRADUATOR_IANGLESINK_H
#define CLEANGRADUATOR_IANGLESINK_H

namespace domain::common {
    class Timestamp;
    class Angle;
}

namespace domain::ports {
    struct IAngleSink {
        virtual ~IAngleSink() = default;
        virtual void onAngle(const common::Timestamp&, const common::Angle&) noexcept = 0;
    };
}

#endif //CLEANGRADUATOR_IANGLESINK_H