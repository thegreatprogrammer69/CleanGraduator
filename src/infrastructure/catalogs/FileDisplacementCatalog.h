#ifndef CLEANGRADUATOR_FILEDISPLACEMENTCATALOG_H
#define CLEANGRADUATOR_FILEDISPLACEMENTCATALOG_H

#include <string>
#include "application/ports/outbound/IDisplacementCatalog.h"

namespace infra::catalogs {
    class FileDisplacementCatalog final : public application::ports::IDisplacementCatalog {
    public:
        explicit FileDisplacementCatalog(std::string filePath);
        std::vector<application::models::Displacement> list() const override;
        std::optional<application::models::Displacement> at(int idx) const override;

    private:
        std::vector<application::models::Displacement> displacements_;
    };
}
#endif //CLEANGRADUATOR_FILEDISPLACEMENTCATALOG_H