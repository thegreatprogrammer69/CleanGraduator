#ifndef CLEANGRADUATOR_FMT_APPLICATION_H
#define CLEANGRADUATOR_FMT_APPLICATION_H

#include <iosfwd>

#include "application/models/info/Displacement.h"
#include "application/models/info/Gauge.h"
#include "application/models/info/Precision.h"
#include "application/models/info/PressureUnit.h"
#include "application/models/info/Printer.h"

namespace application::models {
    std::ostream& operator<<(std::ostream& os, const Displacement& f);
    std::ostream& operator<<(std::ostream& os, const Gauge& f);
    std::ostream& operator<<(std::ostream& os, const Precision& f);
    std::ostream& operator<<(std::ostream& os, const PressureUnit& f);
    std::ostream& operator<<(std::ostream& os, const Printer& f);
}

#endif //CLEANGRADUATOR_FMT_APPLICATION_H
