#include "LogSourcesStorage.h"
#include <algorithm>
#include <optional>

#include "application/ports/outbound/logging/ILogSource.h"

void infra::storage::LogSourcesStorage::add(application::models::LogSource source) {
    sources_.emplace(source.name, source);
}

std::vector<std::string> infra::storage::LogSourcesStorage::all() const {
    std::vector<std::string> names;
    names.reserve(sources_.size());

    for (const auto& [name, _] : sources_)
        names.push_back(name);

    return names;
}

std::optional<application::models::LogSource> infra::storage::LogSourcesStorage::at(const std::string &name) const {
    auto it = sources_.find(name);
    return it != sources_.end() ? it->second : std::nullopt;
}
