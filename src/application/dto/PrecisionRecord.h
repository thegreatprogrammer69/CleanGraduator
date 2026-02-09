#ifndef CLEANGRADUATOR_PRECISIONRECORD_H
#define CLEANGRADUATOR_PRECISIONRECORD_H

namespace application::dto {
    struct PrecisionRecord {
        double precision;
    };
    struct PrecisionRecordId {
        int value;
        bool operator==(const PrecisionRecordId& o) const {  return value == o.value;  }
        bool operator<(const PrecisionRecordId& o) const {  return value < o.value;  }
    };
}

#endif //CLEANGRADUATOR_PRECISIONRECORD_H