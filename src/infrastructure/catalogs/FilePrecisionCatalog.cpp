#include "FilePrecisionCatalog.h"

#include <algorithm>
#include <fstream>
#include <locale>
#include <sstream>
#include <stdexcept>

namespace infra::catalogs {

using application::models::Precision;

FilePrecisionCatalog::FilePrecisionCatalog(std::string filePath) {
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

        try {
            std::replace(line.begin(), line.end(), L',', L'.');
            precisions_.push_back(Precision{.precision = std::stod(line)});
        } catch (const std::exception&) {
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
