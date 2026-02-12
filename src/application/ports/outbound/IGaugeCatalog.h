#ifndef CLEANGRADUATOR_IGAUGECATALOG_H
#define CLEANGRADUATOR_IGAUGECATALOG_H
#include <optional>
#include <vector>
#include "../../../dto/common/GaugeInfo.h"

namespace application::ports {
    struct IGaugeCatalog {
        virtual std::vector<dto::GaugeInfo> list() const = 0;
        virtual std::optional<dto::GaugeInfo> at(dto::GaugeRecordId idx) const = 0;
        virtual ~IGaugeCatalog() = default;
    };
}

#endif //CLEANGRADUATOR_IGAUGECATALOG_H