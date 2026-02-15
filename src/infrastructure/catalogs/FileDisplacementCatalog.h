#ifndef CLEANGRADUATOR_FILEDISPLACEMENTCATALOG_H
#define CLEANGRADUATOR_FILEDISPLACEMENTCATALOG_H

#include <string>
#include "application/ports/outbound/catalogs/IDisplacementCatalog.h"
#include "domain/fmt/Logger.h"

namespace domain::ports {
    struct ILogger;
}

namespace infra::catalogs {
    struct FileDisplacementCatalogPorts {
        domain::ports::ILogger& logger;
    };

    class FileDisplacementCatalog final : public application::ports::IDisplacementCatalog {
    public:
        explicit FileDisplacementCatalog(FileDisplacementCatalogPorts ports, std::string filePath);
        std::vector<application::models::Displacement> list() const override;
        std::optional<application::models::Displacement> at(int idx) const override;

    private:
        std::vector<application::models::Displacement> displacements_;
        fmt::Logger logger_;
    };
}
#endif //CLEANGRADUATOR_FILEDISPLACEMENTCATALOG_H