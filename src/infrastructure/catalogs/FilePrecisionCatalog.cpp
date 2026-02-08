#include "FilePrecisionCatalog.h"

#include <fstream>
#include <sstream>
#include <locale>
#include <algorithm>
#include <stdexcept>

namespace infra::catalogs {

    using application::model::PrecisionRecord;

    FilePrecisionCatalog::FilePrecisionCatalog(std::string filePath) {
        std::wifstream file(filePath);
        if (!file.is_open())
            throw std::runtime_error("failed to open file: " + filePath);

        file.imbue(std::locale(""));

        std::wstring line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            if (!line.empty() && line[0] == L'#') continue; // комментарий

            try {
                // допускаем ',' и '.'
                std::replace(line.begin(), line.end(), L',', L'.');

                PrecisionRecord record;
                record.precision = std::stod(line);

                precisions_.push_back(std::move(record));
            }
            catch (const std::exception&) {
                // битая строка — пропускаем
            }
        }
    }

    std::vector<PrecisionRecord> FilePrecisionCatalog::list() const {
        return precisions_;
    }

    std::optional<PrecisionRecord> FilePrecisionCatalog::at(int idx) const {
        if (idx >= 0 && idx < static_cast<int>(precisions_.size())) {
            return precisions_.at(idx);
        }
        return std::nullopt;
    }

}
