#ifndef CLEANGRADUATOR_FILEPRESSUREUNITCATALOG_H
#define CLEANGRADUATOR_FILEPRESSUREUNITCATALOG_H

#include <string>
#include <vector>
#include <optional>

#include "CatalogPorts.h"
#include "application/ports/catalogs/IPressureUnitCatalog.h"
#include "domain/fmt/Logger.h"

namespace domain::ports {
    struct ILogger;
}

namespace infra::catalogs {

    class FilePressureUnitCatalog final
        : public application::ports::IPressureUnitCatalog {
    public:
        explicit FilePressureUnitCatalog(CatalogPorts ports, std::string filePath);

        std::vector<application::models::PressureUnit> list() const override;
        std::optional<application::models::PressureUnit> at(int idx) const override;

    private:
        std::vector<application::models::PressureUnit> units_;
        fmt::Logger logger_;
    };

}

#endif // CLEANGRADUATOR_FILEPRESSUREUNITCATALOG_H
