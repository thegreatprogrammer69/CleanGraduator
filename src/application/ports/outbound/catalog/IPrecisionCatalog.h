#ifndef CLEANGRADUATOR_IPRECISIONCATALOG_H
#define CLEANGRADUATOR_IPRECISIONCATALOG_H
#include <optional>
#include <vector>
#include "application/model/PrecisionRecord.h"

namespace application::ports {
    struct IPrecisionCatalog {
        virtual std::vector<model::PrecisionRecord> list() const = 0;
        virtual std::optional<model::PrecisionRecord> at(int idx) const = 0;
        virtual ~IPrecisionCatalog() = default;
    };
}

#endif //CLEANGRADUATOR_IPRECISIONCATALOG_H