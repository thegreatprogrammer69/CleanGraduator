#ifndef CLEANGRADUATOR_FILEPRINTERCATALOG_H
#define CLEANGRADUATOR_FILEPRINTERCATALOG_H

#include <string>
#include <vector>
#include "../../application/ports/outbound/catalogs/IPrinterCatalog.h"
#include "domain/fmt/Logger.h"

namespace domain::ports {
    struct ILogger;
}

namespace infra::catalogs {
    struct FilePrinterCatalogPorts {
        domain::ports::ILogger& logger;
    };

    class FilePrinterCatalog final : public application::ports::IPrinterCatalog {
    public:
        explicit FilePrinterCatalog(FilePrinterCatalogPorts ports, std::string filePath);
        std::vector<application::models::Printer> list() const override;
        std::optional<application::models::Printer> at(int idx) const override;

    private:
        std::vector<application::models::Printer> printers_;
        fmt::Logger logger_;
    };
}

#endif // CLEANGRADUATOR_FILEPRINTERCATALOG_H
