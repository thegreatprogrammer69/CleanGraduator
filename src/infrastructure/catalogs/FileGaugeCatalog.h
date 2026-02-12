#ifndef CLEANGRADUATOR_FILEGAUGECATALOG_H
#define CLEANGRADUATOR_FILEGAUGECATALOG_H

#include <string>
#include <vector>
#include "application/ports/outbound/IGaugeCatalog.h"

namespace infra::catalogs {
    class FileGaugeCatalog final : public application::ports::IGaugeCatalog {
    public:
        explicit FileGaugeCatalog(std::string filePath);
        std::vector<application::models::Gauge> list() const override;
        std::optional<application::models::Gauge> at(int idx) const override;

    private:
        std::vector<application::models::Gauge> gauges_;
    };
}

#endif // CLEANGRADUATOR_FILEGAUGECATALOG_H
