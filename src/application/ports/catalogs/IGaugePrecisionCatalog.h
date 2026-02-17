#ifndef CLEANGRADUATOR_IPRECISIONCATALOG_H
#define CLEANGRADUATOR_IPRECISIONCATALOG_H

#include <optional>
#include <vector>

#include "application/models/info/GaugePrecision.h"

namespace application::ports {
    struct IGaugePrecisionCatalog {
        virtual std::vector<models::GaugePrecision> list() const = 0;
        virtual std::optional<models::GaugePrecision> at(int idx) const = 0;
        virtual ~IGaugePrecisionCatalog() = default;
    };
}

#endif // CLEANGRADUATOR_IPRECISIONCATALOG_H
