#include "FileDisplacementCatalog.h"

#include <fstream>
#include <locale>
#include <sstream>
#include <string>
#include <vector>

namespace infra::catalogs {

using application::models::Displacement;

FileDisplacementCatalog::FileDisplacementCatalog(std::string filePath) {
    std::wifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("failed to open file: " + filePath);
    }

    file.imbue(std::locale(""));

    std::wstring line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == L'#') {
            continue;
        }

        std::wistringstream iss(line);
        std::wstring idPart;
        std::wstring namePart;

        if (!std::getline(iss, idPart, L';') || !std::getline(iss, namePart)) {
            continue;
        }

        try {
            Displacement displacement;
            displacement.id = std::stoi(idPart);
            displacement.name = std::move(namePart);
            displacements_.push_back(std::move(displacement));
        } catch (const std::exception&) {
        }
    }
}

std::vector<Displacement> FileDisplacementCatalog::list() const {
    return displacements_;
}

std::optional<Displacement> FileDisplacementCatalog::at(int idx) const {
    if (idx >= 0 && idx < static_cast<int>(displacements_.size())) {
        return displacements_.at(idx);
    }
    return std::nullopt;
}

} // namespace infra::catalogs
