#ifndef CLEANGRADUATOR_IPRINTERCATALOG_H
#define CLEANGRADUATOR_IPRINTERCATALOG_H
#include <optional>
#include <vector>
#include "application/dto/PrinterRecord.h"

namespace application::ports {
    struct IPrinterCatalog {
        virtual std::vector<dto::PrinterRecord> list() const = 0;
        virtual std::optional<dto::PrinterRecord> at(dto::PrinterRecordId idx) const = 0;
        virtual ~IPrinterCatalog() = default;
    };
}

#endif //CLEANGRADUATOR_IPRINTERCATALOG_H