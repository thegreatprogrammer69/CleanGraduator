#include "FileDisplacementCatalog.h"

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <locale>

namespace infra::catalogs {

    using application::model::DisplacementRecord;

    FileDisplacementCatalog::FileDisplacementCatalog(std::string filePath) {
        std::wifstream file(filePath);
        if (!file.is_open())
            throw std::runtime_error("failed to open file: " + filePath);

        // Чтобы корректно читать wide-строки
        file.imbue(std::locale(""));

        std::wstring line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            if (!line.empty() && line[0] == L'#') continue; // комментарий

            std::wistringstream iss(line);

            std::wstring idPart;
            std::wstring namePart;

            if (!std::getline(iss, idPart, L';'))
                continue;

            if (!std::getline(iss, namePart))
                continue;

            try {
                DisplacementRecord record;
                record.id = std::stoi(idPart);
                record.name = std::move(namePart);

                displacements_.push_back(std::move(record));
            }
            catch (const std::exception&) {
                // битая строка — пропускаем
            }
        }
    }

    std::vector<DisplacementRecord> FileDisplacementCatalog::list() const {
        return displacements_;
    }

    std::optional<DisplacementRecord> FileDisplacementCatalog::at(int idx) const {
        if (idx >= 0 && idx < displacements_.size()) {
            return displacements_.at(idx);
        }
        return std::nullopt;
    }
} // namespace infrastructure::catalogs
