#ifndef CLEANGRADUATOR_DISPLACEMENTRECORD_H
#define CLEANGRADUATOR_DISPLACEMENTRECORD_H
#include <string>

namespace application::dto {
    struct DisplacementRecord {
        std::wstring name;
        int code;
    };

    struct DisplacementRecordId {
        int value;
        bool operator==(const DisplacementRecordId& o) const {  return value == o.value;  }
        bool operator<(const DisplacementRecordId& o) const {  return value < o.value;  }
    };

}

#endif //CLEANGRADUATOR_DISPLACEMENTRECORD_H