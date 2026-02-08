#ifndef CLEANGRADUATOR_FILEPRECISIONCATALOG_H
#define CLEANGRADUATOR_FILEPRECISIONCATALOG_H

#include <string>
#include <vector>
#include <optional>

#include "application/ports/outbound/catalog/IPrecisionCatalog.h"

namespace infra::catalogs {

    class FilePrecisionCatalog final
        : public application::ports::IPrecisionCatalog {
    public:
        explicit FilePrecisionCatalog(std::string filePath);

        std::vector<application::model::PrecisionRecord> list() const override;
        std::optional<application::model::PrecisionRecord> at(int idx) const override;

    private:
        std::vector<application::model::PrecisionRecord> precisions_;
    };

}

#endif // CLEANGRADUATOR_FILEPRECISIONCATALOG_H
