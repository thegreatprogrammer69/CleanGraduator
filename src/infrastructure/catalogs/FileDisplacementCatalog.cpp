#include "FileDisplacementCatalog.h"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <codecvt>
#include <fstream>
#include <locale>
#include <stdexcept>
#include <string>

#include "application/fmt/fmt_application.h"

namespace {
std::wstring from_utf8(std::string_view s) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
    return conv.from_bytes(s.data(), s.data() + s.size());
}

void trim(std::string& s) {
    auto notSpace = [](unsigned char ch) { return !std::isspace(ch); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
    s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
}

std::string trim_copy(std::string s) {
    trim(s);
    return s;
}
}

namespace infra::catalogs {

using application::models::Displacement;

FileDisplacementCatalog::FileDisplacementCatalog(FileDisplacementCatalogPorts ports, std::string filePath)
    : logger_(ports.logger)
{
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        logger_.error("Failed to open displacement catalog file: {}", filePath);
        throw std::runtime_error("failed to open file: " + filePath);
    }

    std::string line;
    bool first_line = true;

    while (std::getline(file, line)) {
        if (first_line) {
            first_line = false;
            if (line.size() >= 3 &&
                static_cast<unsigned char>(line[0]) == 0xEF &&
                static_cast<unsigned char>(line[1]) == 0xBB &&
                static_cast<unsigned char>(line[2]) == 0xBF) {
                line.erase(0, 3);
            }
        }

        trim(line);
        if (line.empty() || line[0] == '#') {
            continue;
        }

        const auto separator = line.find(';');
        if (separator == std::string::npos) {
            logger_.error("Failed to parse displacement line: {}", line);
            continue;
        }

        std::string idPart = trim_copy(line.substr(0, separator));
        std::string namePart = trim_copy(line.substr(separator + 1));
        if (idPart.empty() || namePart.empty()) {
            logger_.error("Failed to parse displacement line: {}", line);
            continue;
        }

        int id = 0;
        const auto parsed = std::from_chars(idPart.data(), idPart.data() + idPart.size(), id);
        if (parsed.ec != std::errc() || parsed.ptr != idPart.data() + idPart.size()) {
            logger_.error("Failed to parse displacement id in line: {}", line);
            continue;
        }

        try {
            Displacement d;
            d.id = id;
            d.name = from_utf8(namePart);
            displacements_.push_back(d);
            logger_.info("Loaded displacement model: {}", displacements_.back());
        } catch (const std::range_error&) {
            logger_.error("Invalid UTF-8 in displacement line: {}", line);
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
