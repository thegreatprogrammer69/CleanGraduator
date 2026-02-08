#ifndef CLEANGRADUATOR_PRINTERRECORD_H
#define CLEANGRADUATOR_PRINTERRECORD_H
#include <string>


namespace application::model {
    struct PrinterRecord {
        std::wstring name;
        std::wstring path;
    };
}

#endif //CLEANGRADUATOR_PRINTERRECORD_H