#ifndef CLEANGRADUATOR_ILOGSOURCESTORAGE_H
#define CLEANGRADUATOR_ILOGSOURCESTORAGE_H
#include <optional>
#include <string>
#include <vector>
#include "application/models/logging/LogSource.h"

namespace application::ports {
    struct ILogSourcesStorage {
        virtual ~ILogSourcesStorage() = default;

        virtual std::vector<std::string> all() const = 0;
        virtual std::optional<models::LogSource> at(const std::string& name) const = 0;
    };

}

#endif //CLEANGRADUATOR_ILOGSOURCESTORAGE_H