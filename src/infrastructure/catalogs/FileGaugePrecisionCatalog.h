#ifndef CLEANGRADUATOR_FILEPRECISIONCATALOG_H
#define CLEANGRADUATOR_FILEPRECISIONCATALOG_H

#include <string>
#include <vector>
#include <optional>

#include "CatalogPorts.h"
#include "application/models/info/GaugePrecision.h"
#include "application/ports/catalogs/IGaugePrecisionCatalog.h"
#include "domain/fmt/Logger.h"

namespace domain::ports {
    struct ILogger;
}

namespace infra::catalogs {
    class FileGaugePrecisionCatalog final
        : public application::ports::IGaugePrecisionCatalog {
    public:
        explicit FileGaugePrecisionCatalog(CatalogPorts ports, std::string filePath);

        std::vector<application::models::GaugePrecision> list() const override;
        std::optional<application::models::GaugePrecision> at(int idx) const override;

    private:
        std::vector<application::models::GaugePrecision> precisions_;
        fmt::Logger logger_;
    };

}

#endif // CLEANGRADUATOR_FILEPRECISIONCATALOG_H
