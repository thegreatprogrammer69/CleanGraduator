#ifndef CLEANGRADUATOR_IPRINTERCATALOG_H
#define CLEANGRADUATOR_IPRINTERCATALOG_H

#include <optional>
#include <vector>

#include "application/models/info/Printer.h"

namespace application::ports {
    struct IPrinterCatalog {
        virtual std::vector<models::Printer> list() const = 0;
        virtual std::optional<models::Printer> at(int idx) const = 0;
        virtual ~IPrinterCatalog() = default;
    };
}

#endif // CLEANGRADUATOR_IPRINTERCATALOG_H
