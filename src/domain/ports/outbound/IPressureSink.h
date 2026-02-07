#ifndef CLEANGRADUATOR_IPRESSURESINK_H
#define CLEANGRADUATOR_IPRESSURESINK_H
#include <chrono>
#include "domain/core/measurement/Pressure.h"

namespace domain::common {
    class Timestamp;
    class Pressure;
}


namespace domain::ports {
    struct IPressureSink {
        virtual ~IPressureSink() = default;
        virtual void onPressure(const common::Timestamp&, const common::Pressure&) noexcept = 0;
    };
}

#endif //CLEANGRADUATOR_IPRESSURESINK_H