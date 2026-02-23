#ifndef CLEANGRADUATOR_FAKEPRESSURESOURCECONFIG_H
#define CLEANGRADUATOR_FAKEPRESSURESOURCECONFIG_H

#include <chrono>
#include "domain/core/measurement/Pressure.h"

namespace infra::pressure {

    struct FakePressureSourceConfig {

        enum class Mode {
            Ramp,       // from -> to -> jump to from
            PingPong    // from -> to -> from
        };

        domain::common::Pressure from{};
        domain::common::Pressure to{};

        std::chrono::milliseconds duration{5000};
        std::chrono::milliseconds poll_interval{50};

        Mode mode = Mode::Ramp;
    };

}

#endif