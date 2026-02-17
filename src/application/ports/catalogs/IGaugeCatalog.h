#ifndef CLEANGRADUATOR_IGAUGECATALOG_H
#define CLEANGRADUATOR_IGAUGECATALOG_H

#include <optional>
#include <vector>

#include "application/models/info/Gauge.h"

namespace application::ports {
    struct IGaugeCatalog {
        virtual std::vector<models::Gauge> list() const = 0;
        virtual std::optional<models::Gauge> at(int idx) const = 0;
        virtual ~IGaugeCatalog() = default;
    };
}

#endif // CLEANGRADUATOR_IGAUGECATALOG_H
