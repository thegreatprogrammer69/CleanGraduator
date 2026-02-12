#ifndef CLEANGRADUATOR_IDisplacementCATALOG_H
#define CLEANGRADUATOR_IDisplacementCATALOG_H
#include <optional>
#include <vector>
#include "../../../dto/common/DisplacementRecord.h"

namespace application::ports {
    struct IDisplacementCatalog {
        virtual std::vector<dto::DisplacementRecord> list() const = 0;
        virtual std::optional<dto::DisplacementRecord> at(dto::DisplacementRecordId idx) const = 0;
        virtual ~IDisplacementCatalog() = default;
    };
}

#endif //CLEANGRADUATOR_IDisplacementCATALOG_H