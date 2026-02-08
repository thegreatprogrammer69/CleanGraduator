#ifndef CLEANGRADUATOR_FILEGAUGECATALOG_H
#define CLEANGRADUATOR_FILEGAUGECATALOG_H

#include <string>
#include <vector>
#include "application/ports/outbound/catalog/IGaugeCatalog.h"

namespace infra::catalogs {
    class FileGaugeCatalog final : public application::ports::IGaugeCatalog {
    public:
        explicit FileGaugeCatalog(std::string filePath);
        std::vector<application::model::GaugeRecord> list() const override;
        std::optional<application::model::GaugeRecord> at(int idx) const override;

    private:
        std::vector<application::model::GaugeRecord> gauges_;
    };
}

#endif // CLEANGRADUATOR_FILEGAUGECATALOG_H
