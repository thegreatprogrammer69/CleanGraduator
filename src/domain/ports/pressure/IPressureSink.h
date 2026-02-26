#ifndef CLEANGRADUATOR_IPRESSURESINK_H
#define CLEANGRADUATOR_IPRESSURESINK_H

namespace domain::ports {
    struct IPressureSink {
        virtual ~IPressureSink() = default;
        virtual void onPressure(const common::PressurePacket&) = 0;
    };
}

#endif //CLEANGRADUATOR_IPRESSURESINK_H