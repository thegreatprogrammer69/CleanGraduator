#ifndef CLEANGRADUATOR_FILEPRINTERCATALOG_H
#define CLEANGRADUATOR_FILEPRINTERCATALOG_H

#include <string>
#include <vector>
#include "application/ports/outbound/catalog/IPrinterCatalog.h"

namespace infra::catalogs {
    class FilePrinterCatalog final : public application::ports::IPrinterCatalog {
    public:
        explicit FilePrinterCatalog(std::string filePath);
        std::vector<application::model::PrinterRecord> list() const override;
        std::optional<application::model::PrinterRecord> at(int idx) const override;

    private:
        std::vector<application::model::PrinterRecord> printers_;
    };
}

#endif // CLEANGRADUATOR_FILEPRINTERCATALOG_H
