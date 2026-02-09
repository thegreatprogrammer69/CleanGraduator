#ifndef CLEANGRADUATOR_IPRECISIONCATALOG_H
#define CLEANGRADUATOR_IPRECISIONCATALOG_H
#include <optional>
#include <vector>
#include "application/dto/PrecisionRecord.h"

namespace application::ports {
    struct IPrecisionCatalog {
        virtual std::vector<dto::PrecisionRecord> list() const = 0;
        virtual std::optional<dto::PrecisionRecord> at(dto::PrecisionRecordId idx) const = 0;
        virtual ~IPrecisionCatalog() = default;
    };
}

#endif //CLEANGRADUATOR_IPRECISIONCATALOG_H