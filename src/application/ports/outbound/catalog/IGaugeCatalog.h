#ifndef CLEANGRADUATOR_IGAUGECATALOG_H
#define CLEANGRADUATOR_IGAUGECATALOG_H
#include <optional>
#include <vector>
#include "application/model/GaugeRecord.h"

namespace application::ports {
    struct IGaugeCatalog {
        virtual std::vector<model::GaugeRecord> list() const = 0;
        virtual std::optional<model::GaugeRecord> at(int idx) const = 0;
        virtual ~IGaugeCatalog() = default;
    };
}

#endif //CLEANGRADUATOR_IGAUGECATALOG_H