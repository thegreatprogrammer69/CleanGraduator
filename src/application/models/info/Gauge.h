#ifndef CLEANGRADUATOR_GAUGE_H
#define CLEANGRADUATOR_GAUGE_H

#include <string>
#include "../calibration/GaugePressurePoints.h"

namespace application::models {
    struct Gauge {
        std::wstring name;
        model::GaugePressurePoints points;
    };
}

#endif // CLEANGRADUATOR_GAUGE_H
