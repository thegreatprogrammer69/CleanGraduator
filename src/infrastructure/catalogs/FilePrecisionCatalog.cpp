#include "FilePrecisionCatalog.h"

#include <algorithm>
#include <codecvt>
#include <fstream>
#include <locale>
#include <stdexcept>

#include "application/fmt/fmt_application.h"

namespace {
inline std::string to_utf8(const std::wstring& ws) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
    return conv.to_bytes(ws);
}
}

namespace infra::catalogs {

using application::models::Precision;

FilePrecisionCatalog::FilePrecisionCatalog(FilePrecisionCatalogPorts ports, std::string filePath)
    : logger_(ports.logger)
{
    std::wifstream file(filePath);
    if (!file.is_open()) {
        logger_.error("Failed to open precision catalog file: {}", filePath);
        throw std::runtime_error("failed to open file: " + filePath);
    }

    file.imbue(std::locale(""));

    std::wstring line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == L'#') {
            continue;
        }

        try {
            std::replace(line.begin(), line.end(), L',', L'.');
            precisions_.push_back(Precision{.precision = std::stod(line)});
            logger_.info("Loaded precision model: {}", precisions_.back());
        } catch (const std::exception&) {
            logger_.error("Failed to parse precision line: {}", to_utf8(line));
        }
    }
}

std::vector<Precision> FilePrecisionCatalog::list() const {
    return precisions_;
}

std::optional<Precision> FilePrecisionCatalog::at(int idx) const {
    if (idx >= 0 && idx < static_cast<int>(precisions_.size())) {
        return precisions_.at(idx);
    }
    return std::nullopt;
}

} // namespace infra::catalogs
