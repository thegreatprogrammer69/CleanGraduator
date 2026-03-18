#ifndef CLEANGRADUATOR_CALIBRATIONCONTEXT_H
#define CLEANGRADUATOR_CALIBRATIONCONTEXT_H

#include <string>

#include "application/models/info/Displacement.h"
#include "application/models/info/Gauge.h"
#include "application/models/info/GaugePrecision.h"
#include "application/models/info/PressureUnit.h"
#include "application/models/info/Printer.h"

namespace application::models {
    struct CalibrationContext {
        Displacement displacement;
        Gauge gauge;
        GaugePrecision precision;
        PressureUnit pressure_unit;
        Printer printer;

        std::string batch_path() const { return printer.path; }
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONCONTEXT_H
