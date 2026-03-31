#ifndef CLEANGRADUATOR_GAUGE_H
#define CLEANGRADUATOR_GAUGE_H

#include <string>
#include "../calibration/GaugePressurePoints.h"

namespace application::models {
    struct Gauge {
        std::string name;
        float central_pressure;
        model::GaugePressurePoints points;
    };
}

#endif // CLEANGRADUATOR_GAUGE_H
