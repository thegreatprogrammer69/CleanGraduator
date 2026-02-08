#ifndef CLEANGRADUATOR_GaugeRecord_H
#define CLEANGRADUATOR_GaugeRecord_H
#include <string>
#include <vector>
#include "domain/core/measurement/PressurePoints.h"

namespace application::model {
    struct GaugeRecord {
        std::wstring name;
        domain::common::PressurePoints values;
    };
}

#endif //CLEANGRADUATOR_GaugeRecord_H