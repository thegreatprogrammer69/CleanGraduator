#ifndef CLEANGRADUATOR_IDISPLACEMENTCATALOG_H
#define CLEANGRADUATOR_IDISPLACEMENTCATALOG_H

#include <optional>
#include <vector>

#include "application/models/info/Displacement.h"

namespace application::ports {
    struct IDisplacementCatalog {
        virtual std::vector<models::Displacement> list() const = 0;
        virtual std::optional<models::Displacement> at(int idx) const = 0;
        virtual ~IDisplacementCatalog() = default;
    };
}

#endif // CLEANGRADUATOR_IDISPLACEMENTCATALOG_H
