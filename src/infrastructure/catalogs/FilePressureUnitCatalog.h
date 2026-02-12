#ifndef CLEANGRADUATOR_FILEPRESSUREUNITCATALOG_H
#define CLEANGRADUATOR_FILEPRESSUREUNITCATALOG_H

#include <string>
#include <vector>
#include <optional>

#include "application/ports/outbound/IPressureUnitCatalog.h"

namespace infra::catalogs {

    class FilePressureUnitCatalog final
        : public application::ports::IPressureUnitCatalog {
    public:
        explicit FilePressureUnitCatalog(std::string filePath);

        std::vector<application::models::PressureUnit> list() const override;
        std::optional<application::models::PressureUnit> at(int idx) const override;

    private:
        std::vector<application::models::PressureUnit> units_;
    };

}

#endif // CLEANGRADUATOR_FILEPRESSUREUNITCATALOG_H
