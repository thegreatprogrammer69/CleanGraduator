#ifndef CLEANGRADUATOR_IPRESSUREUNITCATALOG_H
#define CLEANGRADUATOR_IPRESSUREUNITCATALOG_H

#include <optional>
#include <vector>

#include "application/models/info/PressureUnit.h"

namespace application::ports {
    struct IPressureUnitCatalog {
        virtual std::vector<models::PressureUnit> list() const = 0;
        virtual std::optional<models::PressureUnit> at(int idx) const = 0;
        virtual ~IPressureUnitCatalog() = default;
    };
}

#endif // CLEANGRADUATOR_IPRESSUREUNITCATALOG_H
