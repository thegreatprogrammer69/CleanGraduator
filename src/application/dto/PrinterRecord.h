#ifndef CLEANGRADUATOR_PRINTERRECORD_H
#define CLEANGRADUATOR_PRINTERRECORD_H
#include <string>


namespace application::dto {
    struct PrinterRecord {
        std::wstring name;
        std::wstring path;
    };
    struct PrinterRecordId {
        int value;
        bool operator==(const PrinterRecordId& o) const {  return value == o.value;  }
        bool operator<(const PrinterRecordId& o) const {  return value < o.value;  }
    };
}

#endif //CLEANGRADUATOR_PRINTERRECORD_H