#ifndef CLEANGRADUATOR_IGAUGECATALOG_H
#define CLEANGRADUATOR_IGAUGECATALOG_H
#include <optional>
#include <vector>
#include "application/dto/GaugeRecord.h"

namespace application::ports {
    struct IGaugeCatalog {
        virtual std::vector<dto::GaugeRecord> list() const = 0;
        virtual std::optional<dto::GaugeRecord> at(dto::GaugeRecordId idx) const = 0;
        virtual ~IGaugeCatalog() = default;
    };
}

#endif //CLEANGRADUATOR_IGAUGECATALOG_H