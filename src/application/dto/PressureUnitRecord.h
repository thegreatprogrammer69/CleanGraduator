#ifndef CLEANGRADUATOR_PRESSUREUNITRECORD_H
#define CLEANGRADUATOR_PRESSUREUNITRECORD_H
#include "domain/core/measurement/PressureUnit.h"

namespace application::dto {
    struct PressureUnitRecord {
        domain::common::PressureUnit unit;
    };
    struct PressureUnitRecordId {
        int value;
        bool operator==(const PressureUnitRecordId& o) const {  return value == o.value;  }
        bool operator<(const PressureUnitRecordId& o) const {  return value < o.value;  }
    };
}

#endif //CLEANGRADUATOR_PRESSUREUNITRECORD_H