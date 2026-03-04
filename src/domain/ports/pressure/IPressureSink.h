#ifndef CLEANGRADUATOR_IPRESSURESINK_H
#define CLEANGRADUATOR_IPRESSURESINK_H

namespace domain::common {
    struct PressurePacket;
}

namespace domain::ports {
    struct IPressureSink {
        virtual ~IPressureSink() = default;
        virtual void onPressurePacket(const common::PressurePacket&) = 0;
    };
}

#endif //CLEANGRADUATOR_IPRESSURESINK_H