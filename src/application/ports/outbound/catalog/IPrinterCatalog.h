#ifndef CLEANGRADUATOR_IPRINTERCATALOG_H
#define CLEANGRADUATOR_IPRINTERCATALOG_H
#include <optional>
#include <vector>
#include "application/model/PrinterRecord.h"

namespace application::ports {
    struct IPrinterCatalog {
        virtual std::vector<model::PrinterRecord> list() const = 0;
        virtual std::optional<model::PrinterRecord> at(int idx) const = 0;
        virtual ~IPrinterCatalog() = default;
    };
}

#endif //CLEANGRADUATOR_IPRINTERCATALOG_H