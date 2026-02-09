#ifndef CLEANGRADUATOR_IPRESSUREUNITCATALOG_H
#define CLEANGRADUATOR_IPRESSUREUNITCATALOG_H
#include <optional>
#include <vector>
#include "application/dto/PressureUnitRecord.h"

namespace application::ports {
    struct IPressureUnitCatalog {
        virtual std::vector<dto::PressureUnitRecord> list() const = 0;
        virtual std::optional<dto::PressureUnitRecord> at(dto::PressureUnitRecordId idx) const = 0;
        virtual ~IPressureUnitCatalog() = default;
    };
}

#endif //CLEANGRADUATOR_IPRESSUREUNITCATALOG_H