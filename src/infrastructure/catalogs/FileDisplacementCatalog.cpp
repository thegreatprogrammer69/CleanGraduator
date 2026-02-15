#include "FileDisplacementCatalog.h"

#include <codecvt>
#include <fstream>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "application/fmt/fmt_application.h"

namespace {
inline std::string to_utf8(const std::wstring& ws) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
    return conv.to_bytes(ws);
}
}

namespace infra::catalogs {

using application::models::Displacement;

FileDisplacementCatalog::FileDisplacementCatalog(FileDisplacementCatalogPorts ports, std::string filePath)
    : logger_(ports.logger)
{
    std::wifstream file(filePath);
    if (!file.is_open()) {
        logger_.error("Failed to open displacement catalog file: {}", filePath);
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
            logger_.error("Failed to parse displacement line: {}", to_utf8(line));
            continue;
        }

        try {
            Displacement displacement;
            displacement.id = std::stoi(idPart);
            displacement.name = std::move(namePart);
            displacements_.push_back(std::move(displacement));
            logger_.info("Loaded displacement model: {}", displacements_.back());
        } catch (const std::exception&) {
            logger_.error("Failed to convert displacement line: {}", to_utf8(line));
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
