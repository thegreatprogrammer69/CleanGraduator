#ifndef CLEANGRADUATOR_FILEPRINTERCATALOG_H
#define CLEANGRADUATOR_FILEPRINTERCATALOG_H

#include <string>
#include <vector>
#include "../../application/ports/outbound/catalogs/IPrinterCatalog.h"

namespace infra::catalogs {
    class FilePrinterCatalog final : public application::ports::IPrinterCatalog {
    public:
        explicit FilePrinterCatalog(std::string filePath);
        std::vector<application::models::Printer> list() const override;
        std::optional<application::models::Printer> at(int idx) const override;

    private:
        std::vector<application::models::Printer> printers_;
    };
}

#endif // CLEANGRADUATOR_FILEPRINTERCATALOG_H
