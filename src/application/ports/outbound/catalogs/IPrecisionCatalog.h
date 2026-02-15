#ifndef CLEANGRADUATOR_IPRECISIONCATALOG_H
#define CLEANGRADUATOR_IPRECISIONCATALOG_H

#include <optional>
#include <vector>

#include "application/models/info/Precision.h"

namespace application::ports {
    struct IPrecisionCatalog {
        virtual std::vector<models::Precision> list() const = 0;
        virtual std::optional<models::Precision> at(int idx) const = 0;
        virtual ~IPrecisionCatalog() = default;
    };
}

#endif // CLEANGRADUATOR_IPRECISIONCATALOG_H
