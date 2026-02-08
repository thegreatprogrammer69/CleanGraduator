#ifndef CLEANGRADUATOR_IPRESSUREUNITCATALOG_H
#define CLEANGRADUATOR_IPRESSUREUNITCATALOG_H
#include <optional>
#include <vector>
#include "application/model/PressureUnitRecord.h"

namespace application::ports {
    struct IPressureUnitCatalog {
        virtual std::vector<model::PressureUnitRecord> list() const = 0;
        virtual std::optional<model::PressureUnitRecord> at(int idx) const = 0;
        virtual ~IPressureUnitCatalog() = default;
    };
}

#endif //CLEANGRADUATOR_IPRESSUREUNITCATALOG_H