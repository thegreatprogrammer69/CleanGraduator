#ifndef CLEANGRADUATOR_GAUGE_H
#define CLEANGRADUATOR_GAUGE_H

#include <string>
#include "domain/core/calibration/calculation/GaugePressurePoints.h"

namespace application::models {
    struct Gauge {
        std::wstring name;
        domain::common::GaugePressurePoints points;
    };
}

#endif // CLEANGRADUATOR_GAUGE_H
