#ifndef CLEANGRADUATOR_FILEPRECISIONCATALOG_H
#define CLEANGRADUATOR_FILEPRECISIONCATALOG_H

#include <string>
#include <vector>
#include <optional>

#include "../../application/ports/outbound/catalogs/IPrecisionCatalog.h"
#include "domain/fmt/Logger.h"

namespace domain::ports {
    struct ILogger;
}

namespace infra::catalogs {

    struct FilePrecisionCatalogPorts {
        domain::ports::ILogger& logger;
    };

    class FilePrecisionCatalog final
        : public application::ports::IPrecisionCatalog {
    public:
        explicit FilePrecisionCatalog(FilePrecisionCatalogPorts ports, std::string filePath);

        std::vector<application::models::Precision> list() const override;
        std::optional<application::models::Precision> at(int idx) const override;

    private:
        std::vector<application::models::Precision> precisions_;
        fmt::Logger logger_;
    };

}

#endif // CLEANGRADUATOR_FILEPRECISIONCATALOG_H
