#ifndef CLEANGRADUATOR_GaugeRecord_H
#define CLEANGRADUATOR_GaugeRecord_H
#include <string>
#include <vector>
#include "domain/core/calibration/CalibrationPoints.h"

namespace application::dto {
    struct GaugeRecord {
        std::wstring name;
        domain::common::CalibrationPoints values;
    };
    struct GaugeRecordId {
        int value;
        bool operator==(const GaugeRecordId& o) const {  return value == o.value;  }
        bool operator<(const GaugeRecordId& o) const {  return value < o.value;  }
    };
}

#endif //CLEANGRADUATOR_GaugeRecord_H