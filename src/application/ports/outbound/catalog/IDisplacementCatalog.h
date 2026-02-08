#ifndef CLEANGRADUATOR_IDisplacementCATALOG_H
#define CLEANGRADUATOR_IDisplacementCATALOG_H
#include <optional>
#include <vector>
#include "application/model/DisplacementRecord.h"

namespace application::ports {
    struct IDisplacementCatalog {
        virtual std::vector<model::DisplacementRecord> list() const = 0;
        virtual std::optional<model::DisplacementRecord> at(int idx) const = 0;
        virtual ~IDisplacementCatalog() = default;
    };
}

#endif //CLEANGRADUATOR_IDisplacementCATALOG_H