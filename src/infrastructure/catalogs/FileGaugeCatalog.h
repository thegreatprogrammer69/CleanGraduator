#ifndef CLEANGRADUATOR_FILEGAUGECATALOG_H
#define CLEANGRADUATOR_FILEGAUGECATALOG_H

#include <string>
#include <vector>
#include "application/ports/catalogs/IGaugeCatalog.h"
#include "domain/fmt/Logger.h"

namespace domain::ports {
    struct ILogger;
}

namespace infra::catalogs {
    struct FileGaugeCatalogPorts {
        domain::ports::ILogger& logger;
    };

    class FileGaugeCatalog final : public application::ports::IGaugeCatalog {
    public:
        explicit FileGaugeCatalog(FileGaugeCatalogPorts ports, std::string filePath);
        std::vector<application::models::Gauge> list() const override;
        std::optional<application::models::Gauge> at(int idx) const override;

    private:
        std::vector<application::models::Gauge> gauges_;
        fmt::Logger logger_;
    };
}

#endif // CLEANGRADUATOR_FILEGAUGECATALOG_H
