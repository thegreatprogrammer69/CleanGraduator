#ifndef CLEANGRADUATOR_FILEPRECISIONCATALOG_H
#define CLEANGRADUATOR_FILEPRECISIONCATALOG_H

#include <string>
#include <vector>
#include <optional>

#include "application/ports/outbound/IPrecisionCatalog.h"

namespace infra::catalogs {

    class FilePrecisionCatalog final
        : public application::ports::IPrecisionCatalog {
    public:
        explicit FilePrecisionCatalog(std::string filePath);

        std::vector<application::models::Precision> list() const override;
        std::optional<application::models::Precision> at(int idx) const override;

    private:
        std::vector<application::models::Precision> precisions_;
    };

}

#endif // CLEANGRADUATOR_FILEPRECISIONCATALOG_H
